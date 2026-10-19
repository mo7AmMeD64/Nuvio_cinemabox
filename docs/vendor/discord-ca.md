# Discord TLS trust roots

`deploy/app/art/discord-ca.pem` is the unmodified Mozilla CA bundle distributed
by curl, downloaded on 2026-10-03 from https://curl.se/ca/cacert.pem.
The embedded source timestamp is 2026-09-25 03:12:01 GMT.

SHA-256: `a41b5d356aea97a529fe27e0f7316d2f9d946d75927476cf9cf1b90637d00505`.
The downloaded file matches https://curl.se/ca/cacert.pem.sha256.
Size: 188900 bytes. Source and update procedure:
https://curl.se/docs/caextract.html.

Mozilla Public License 2.0 applies; the complete license is in
[MPL-2.0.txt](MPL-2.0.txt), obtained from
https://www.mozilla.org/media/MPL/2.0/index.txt.

Native Discord TLS uses this asset independently of the existing HTTP client.
TV packages must contain it; missing roots fail closed. Desktop development
may use system trust if the asset is absent. Browser WebSocket trust is managed
by the browser. Packaging copies this exact public PEM filename, never arbitrary
PEM files, and rejects Discord profile token files including temporary writes.

To update, download the bundle and its official SHA-256 sidecar, compare the
complete-file digest, preserve the unmodified PEM, and update this provenance.
