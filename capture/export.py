#!/usr/bin/env python3
"""Verify a Mathieu replay and use the canonical Kitchen movie assembler.

The interactive renderer first saves a session and renders frames offline.
Only isomorphisms/kitchen owns the FFmpeg encoding command.
This module does not screen-record, invent motion, or certify Mathieu membership.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
from fractions import Fraction

PNG_HEADER = b"\x89PNG\r\n\x1a\n"
FRAME_NAME = "frame_{:06d}.png"
HEX_SHA = re.compile(r"[0-9a-f]{64}\Z")
SOURCE_SHA = re.compile(r"[0-9a-f]{40}\Z")
GROUP_DEGREES = {"M11": 11, "M12": 12, "M22": 22, "M23": 23, "M24": 24}


class CaptureError(Exception):
    """Invalid session, corrupt frame source, or unusable video."""


def positive_integer(value, field):
    if type(value) is not int or value < 1:
        raise CaptureError(f"{field} must be a positive integer")
    return value


def permutation(value, degree, field):
    if not isinstance(value, list) or len(value) != degree:
        raise CaptureError(f"{field} must be a permutation of {degree} points")
    if any(type(point) is not int for point in value) or sorted(value) != list(range(degree)):
        raise CaptureError(f"{field} is not a bijection on {degree} points")
    return value


def validate_math(math):
    if not isinstance(math, dict):
        raise CaptureError("math must contain exact action and generator data")
    group = math.get("group")
    if group not in GROUP_DEGREES:
        raise CaptureError("math.group must be one of M11, M12, M22, M23, M24")
    degree = GROUP_DEGREES[group]
    generators = math.get("generators")
    word = math.get("word")
    if not isinstance(generators, dict) or not generators:
        raise CaptureError("math.generators must contain exact permutations")
    if not isinstance(word, list) or not word or any(not isinstance(x, str) for x in word):
        raise CaptureError("math.word must be a nonempty list of generator names")
    for name, mapping in generators.items():
        if not isinstance(name, str) or not name:
            raise CaptureError("generator names must be nonempty strings")
        permutation(mapping, degree, f"math.generators.{name}")
    result = list(range(degree))
    for name in word:
        if name not in generators:
            raise CaptureError(f"unknown generator {name!r}")
        # Word acts left to right, i.e. the next generator acts on current images.
        result = [generators[name][point] for point in result]
    final = permutation(math.get("final_mapping"), degree, "math.final_mapping")
    if result != final:
        raise CaptureError("math.final_mapping differs from the recorded generator word")
    # These checks establish consistency, NOT that the given generators are
    # authentic generators of a Mathieu group. That requires external fixtures.


def validate_manifest(manifest):
    if not isinstance(manifest, dict) or manifest.get("format") != "mathieu.capture.v1":
        raise CaptureError("expected format mathieu.capture.v1")
    width = positive_integer(manifest.get("width"), "width")
    height = positive_integer(manifest.get("height"), "height")
    fps = positive_integer(manifest.get("fps"), "fps")
    if width % 2 or height % 2 or width > 7680 or height > 4320:
        raise CaptureError("H.264 yuv420p needs even dimensions no larger than 7680x4320")
    if fps not in (24, 30, 60):
        raise CaptureError("fps must be 24, 30, or 60")
    if not isinstance(manifest.get("source_revision"), str) or not SOURCE_SHA.fullmatch(manifest["source_revision"]):
        raise CaptureError("source_revision must be a 40-digit lowercase commit SHA")
    hashes = manifest.get("frame_sha256")
    if not isinstance(hashes, list) or not 1 <= len(hashes) <= 999999:
        raise CaptureError("frame_sha256 must describe 1 to 999999 consecutive frames")
    if any(not isinstance(h, str) or not HEX_SHA.fullmatch(h) for h in hashes):
        raise CaptureError("each frame_sha256 value must be a lowercase SHA-256")
    events = manifest.get("events")
    if not isinstance(events, list) or not events:
        raise CaptureError("events must contain the original interaction")
    previous = -1
    for event in events:
        if not isinstance(event, dict) or type(event.get("frame")) is not int:
            raise CaptureError("each event needs an integer frame index")
        frame = event["frame"]
        if frame < previous or frame < 0 or frame >= len(hashes):
            raise CaptureError("interaction events must be ordered and within the recording")
        if not isinstance(event.get("kind"), str) or not event["kind"]:
            raise CaptureError("each event needs a nonempty kind")
        previous = frame
    validate_math(manifest.get("math"))
    return width, height, fps, hashes


def verify_frames(directory, width, height, hashes):
    if not directory.is_dir():
        raise CaptureError(f"frame directory not found: {directory}")
    expected = {FRAME_NAME.format(i) for i in range(len(hashes))}
    actual = {p.name for p in directory.glob("frame_*.png")}
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        raise CaptureError(f"nonconsecutive frames (missing={missing[:3]}, extra={extra[:3]})")
    for index, expected_sha in enumerate(hashes):
        path = directory / FRAME_NAME.format(index)
        if path.is_symlink() or not path.is_file():
            raise CaptureError(f"frame is not a regular file: {path}")
        with path.open("rb") as stream:
            header = stream.read(24)
        if len(header) != 24 or header[:8] != PNG_HEADER or header[12:16] != b"IHDR":
            raise CaptureError(f"invalid PNG header: {path}")
        found_width, found_height = struct.unpack(">II", header[16:24])
        if (found_width, found_height) != (width, height):
            raise CaptureError(f"wrong PNG size: {path} {found_width}x{found_height}")
        digest = hashlib.sha256()
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1 << 20), b""):
                digest.update(block)
        if digest.hexdigest() != expected_sha:
            raise CaptureError(f"frame checksum mismatch: {path}")


def command(argv):
    try:
        return subprocess.run(argv, check=True, capture_output=True, text=True).stdout
    except FileNotFoundError as exc:
        raise CaptureError(f"required command not found: {argv[0]}") from exc
    except subprocess.CalledProcessError as exc:
        raise CaptureError(f"{argv[0]} failed: {exc.stderr[-3000:]}") from exc


def verify_video(path, width, height, fps, count):
    details = json.loads(command([
        "ffprobe", "-v", "error", "-select_streams", "v:0", "-count_frames",
        "-show_entries", "stream=codec_name,width,height,pix_fmt,avg_frame_rate,nb_read_frames",
        "-show_entries", "format=duration", "-of", "json", str(path)
    ]))
    streams = details.get("streams", [])
    if len(streams) != 1:
        raise CaptureError("export must have exactly one video stream")
    stream = streams[0]
    if (stream.get("codec_name") != "h264"
        or stream.get("pix_fmt") != "yuv420p"
        or stream.get("width") != width
        or stream.get("height") != height
        or Fraction(stream.get("avg_frame_rate", "0")) != fps
        or stream.get("nb_read_frames") != str(count)):
        raise CaptureError(f"video metadata or decoded frame count differs: {stream}")
    duration = float(details.get("format", {}).get("duration", 0))
    if abs(duration - count / fps) > 1 / fps / 10:
        raise CaptureError(f"video duration {duration} differs from {count / fps}")


def export(session_path, frames_dir, output):
    manifest = json.loads(session_path.read_text(encoding="utf-8"))
    width, height, fps, hashes = validate_manifest(manifest)
    verify_frames(frames_dir, width, height, hashes)
    if output.suffix.lower() != ".mp4":
        raise CaptureError("output must end in .mp4")

    # The movie assembly contract belongs to isomorphisms/kitchen.
    # This Mathieu wrapper validates mathematical state and inputs, then
    # invokes the pinned canonical builder without duplicating FFmpeg flags.
    kitchen_root = os.environ.get("KITCHEN_ROOT")
    if not kitchen_root:
        raise CaptureError("KITCHEN_ROOT must point to a pinned isomorphisms/kitchen checkout")
    kitchen_builder = Path(kitchen_root) / "tasks" / "movie-from-stills" / "build.sh"
    if not kitchen_builder.is_file():
        raise CaptureError(f"canonical Kitchen movie builder missing: {kitchen_builder}")

    output.parent.mkdir(parents=True, exist_ok=True)
    # Keep old public output intact on encoder or verification failure.
    with tempfile.NamedTemporaryFile(
        prefix=".mathieu-export-", suffix=".mp4", dir=output.parent, delete=False
    ) as stream:
        pending = Path(stream.name)
    try:
        command([
            "sh", str(kitchen_builder), str(frames_dir / "frame_%06d.png"),
            str(fps), str(pending),
        ])
        verify_video(pending, width, height, fps, len(hashes))
        os.replace(pending, output)
    finally:
        pending.unlink(missing_ok=True)
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("session", type=Path, help="session.json written by replay renderer")
    parser.add_argument("frames", type=Path, help="directory of exact numbered PNG frames")
    parser.add_argument("output", type=Path, help="verified H.264 MP4 output")
    args = parser.parse_args()
    try:
        result = export(args.session, args.frames, args.output)
        print(f"Verified video: {result}")
        return 0
    except (CaptureError, ValueError, OSError, json.JSONDecodeError) as exc:
        print(f"Mathieu capture export failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
