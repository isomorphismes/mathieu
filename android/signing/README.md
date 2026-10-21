# Permanent public development signer

Authorized by the user on 2026-10-21: “Make one”.

`mathieu-development.p12` is the permanent, deliberately public development key for `org.isomorphisms.mathieu`. It is not a production/store key. Reuse these exact bytes for every sideloaded development update; never regenerate the key during a build.

Format: PKCS12. Alias: `mathieu-development`. Store and key passwords: `android` (public test credentials). RSA 3072, SHA256withRSA, validity 10,000 days from creation.

Certificate SHA-256: `f997a30170a967e073ab47e1212a71b3967ffe2e1e88d78d659c77bf085cb3fe`.

Keystore SHA-256: `00ecef16e04031009904028d6c1e411110bfcc2fb8486223a7835259567a3df7`.

The DER file contains the same certificate exported from the keystore. The maintained android-NDK packager must verify this certificate digest before packaging and again on the finished APK. Keep the package ID stable and versionCode nondecreasing. Never uninstall to bypass an update conflict.
