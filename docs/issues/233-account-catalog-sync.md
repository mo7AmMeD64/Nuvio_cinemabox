# Issue 233: account catalogue configuration

The synthetic account fixture reproduced two shared parser defects: only the
first 64 entries were read before sorting, so later low-order and disabled
catalogues were ignored; an explicit `items: []` retained previous remote state.

The parser now retains up to 768 entries, matching the local row registry, and
recognizes explicit empty modern configuration. Scratch storage is static to
avoid increasing the TV thread stack. Application remains on the main thread.
Missing configuration and empty RPC results retain existing settings. Unknown
local catalogues still append to remote order; explicit local TV choices retain
precedence. An empty addon response still cannot remove all installed addons.

Validation: a 300-entry reverse-order fixture failed before and passes after,
including a disabled last entry and repeated explicit clear. Catalogue parser
(normal and ASan/UBSan), catalogue cache and sync-order checks passed.

This establishes parser defects, not a complete reproduction of the reporter's
Samsung account. Native Tizen device validation and the reporter's actual sync
response remain outstanding. No account data was changed and no release was
published for this fix.
