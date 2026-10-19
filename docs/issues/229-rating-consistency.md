# IMDb rating consistency (#229)

Home, expanded cards and See all previously read the catalog score, while detail preferred IMDb from MDBList. MDBList did not publish that verified result back to other views. TMDB discovery items also carry vote_average in the same catalog field; dynamic Home and See all could present that value with IMDb styling.

All four views now share an identity-bound IMDb rating lookup. Verified MDBList IMDb results are retained in a bounded session cache of 256 entries; no network requests or allocation occur during rendering. IMDb episode identities share their base title. A TMDB alias is published only when the existing metadata request has an explicit IMDb/TMDB association, with movie and TV namespaces separated even when numeric IDs match. An unresolved TMDB catalog score is never an IMDb fallback. CatItem and disk-cache formats are unchanged.

Until a verified IMDb result is fetched through existing detail enrichment, a compatible catalog fallback may still be stale. Cache entries disappear on process exit or bounded eviction. This fixes demonstrated internal inconsistencies; it does not establish the cause of the reporter’s particular screenshot or prove the behavior on their Samsung TV.

Validation: tests/imdbnota.sh passed normally and under ASAN/UBSAN, covering catalog 7.2→verified8.1, separate titles, episode aliases, unresolved TMDB exclusion, explicit TV alias without movie collision, invalid results, and bounded eviction. Full-core compilation and ondever harness passed; Home trailer timer, extras-hero and trailer-serie harnesses passed. These are host checks, not physical TV validation.
