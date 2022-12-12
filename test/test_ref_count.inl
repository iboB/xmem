// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

TEST_CASE("ref count") {
    ref_count_type rc;
    CHECK(rc.count() == 1);
    CHECK(rc.inc() == 2);
    CHECK(rc.count() == 2);
    CHECK(rc.dec() == 1);
    CHECK(rc.count() == 1);
    CHECK(rc.inc_nz() == 2);
    CHECK(rc.count() == 2);
    CHECK(rc.dec() == 1);
    CHECK(rc.dec() == 0);
    CHECK(rc.inc_nz() == 0);
    CHECK(rc.count() == 0);
}

