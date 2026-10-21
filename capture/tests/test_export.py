"""Contract and end-to-end tests for the host-side Mathieu video exporter.

Fixture uses the identity action: this does NOT qualify Mathieu generators.
"""
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest
import zlib

from capture.export import CaptureError, export, validate_manifest


def chunk(kind, payload):
    data = kind + payload
    return struct.pack(">I", len(payload)) + data + struct.pack(">I", zlib.crc32(data) & 0xffffffff)


def png(width, height, rgb):
    raw = b"".join(b"\x00" + bytes(rgb) * width for _ in range(height))
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(raw))
        + chunk(b"IEND", b"")
    )


class MathieuExportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.frames = self.root / "frames"
        self.frames.mkdir()
        self.manifest = {
            "format": "mathieu.capture.v1",
            "width": 64,
            "height": 48,
            "fps": 60,
            "source_revision": "a" * 40,
            "events": [
                {"frame": 0, "kind": "touch_down", "point": [5, 5]},
                {"frame": 2, "kind": "touch_move", "point": [8, 8]},
                {"frame": 3, "kind": "touch_up", "point": [8, 8]},
            ],
            "math": {
                "group": "M11",
                "generators": {"identity": list(range(11))},
                "word": ["identity"],
                "final_mapping": list(range(11)),
            },
            "frame_sha256": [],
        }
        colors = [(230, 15, 5), (230, 15, 5), (5, 25, 225), (5, 25, 225)]
        for i, color in enumerate(colors):
            data = png(64, 48, color)
            (self.frames / f"frame_{i:06d}.png").write_bytes(data)
            self.manifest["frame_sha256"].append(hashlib.sha256(data).hexdigest())
        self.session = self.root / "session.json"
        self.save()

    def save(self):
        self.session.write_text(json.dumps(self.manifest), encoding="utf-8")

    def test_manifest(self):
        width, height, fps, hashes = validate_manifest(self.manifest)
        self.assertEqual((width, height, fps, len(hashes)), (64, 48, 60, 4))

    def test_bad_mapping_rejected(self):
        self.manifest["math"]["final_mapping"] = list(reversed(range(11)))
        with self.assertRaisesRegex(CaptureError, "differs"):
            validate_manifest(self.manifest)

    def test_nonbijective_generator_rejected(self):
        self.manifest["math"]["generators"]["identity"][0] = 1
        with self.assertRaisesRegex(CaptureError, "bijection"):
            validate_manifest(self.manifest)

    def test_event_order_rejected(self):
        self.manifest["events"][1]["frame"] = 3
        self.manifest["events"][2]["frame"] = 2
        with self.assertRaisesRegex(CaptureError, "ordered"):
            validate_manifest(self.manifest)

    def test_missing_frame_rejected(self):
        (self.frames / "frame_000002.png").unlink()
        with self.assertRaisesRegex(CaptureError, "nonconsecutive"):
            export(self.session, self.frames, self.root / "out.mp4")

    def test_tampered_frame_rejected(self):
        (self.frames / "frame_000001.png").write_bytes(png(64, 48, (1, 1, 1)))
        with self.assertRaisesRegex(CaptureError, "checksum"):
            export(self.session, self.frames, self.root / "out.mp4")

    def test_wrong_frame_size_rejected(self):
        data = png(128, 48, (0, 0, 0))
        (self.frames / "frame_000001.png").write_bytes(data)
        self.manifest["frame_sha256"][1] = hashlib.sha256(data).hexdigest()
        self.save()
        with self.assertRaisesRegex(CaptureError, "wrong PNG size"):
            export(self.session, self.frames, self.root / "out.mp4")

    @unittest.skipUnless(shutil.which("ffmpeg") and shutil.which("ffprobe"), "needs ffmpeg and ffprobe")
    def test_mp4_has_all_frames_and_distinct_ends(self):
        video = self.root / "out.mp4"
        self.assertEqual(export(self.session, self.frames, video), video)
        self.assertGreater(video.stat().st_size, 200)
        decoded = subprocess.run([
            "ffmpeg", "-nostdin", "-loglevel", "error", "-i", str(video),
            "-f", "rawvideo", "-pix_fmt", "rgb24", "-"
        ], check=True, capture_output=True).stdout
        frame_bytes = 64 * 48 * 3
        self.assertEqual(len(decoded), frame_bytes * 4)
        first = decoded[frame_bytes // 2:frame_bytes // 2 + 3]
        last = decoded[3 * frame_bytes + frame_bytes // 2:3 * frame_bytes + frame_bytes // 2 + 3]
        self.assertGreater(first[0], first[2] + 80)
        self.assertGreater(last[2], last[0] + 80)


if __name__ == "__main__":
    unittest.main()
