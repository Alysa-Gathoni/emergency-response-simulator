#include "Check.h"
#include "ui/Hud.h"
#include "ui/SevenSegment.h"

namespace {

void segmentMasks() {
    CHECK(SevenSegment::maskFor('8') == 0x7F);      // all seven lit
    CHECK(SevenSegment::maskFor('1') == 0x06);      // b and c
    CHECK(SevenSegment::maskFor('0') == 0x3F);      // everything but g
    CHECK(SevenSegment::maskFor('-') == 0x40);      // g only
    CHECK(SevenSegment::maskFor('e') == SevenSegment::maskFor('E'));
    CHECK(SevenSegment::maskFor('?') == 0);         // unsupported -> blank
}

void segmentCountsMatchMasks() {
    CHECK(SevenSegment::segments('8', {0, 0}, 20, 3).size() == 7);
    CHECK(SevenSegment::segments('1', {0, 0}, 20, 3).size() == 2);
    CHECK(SevenSegment::segments(':', {0, 0}, 20, 3).size() == 2);
    // Every segment is a hexagon.
    for (const auto& seg : SevenSegment::segments('8', {0, 0}, 20, 3))
        CHECK(seg.size() == 6);
}

void clockFormatting() {
    CHECK(Hud::clock(0.0f) == "00:00");
    CHECK(Hud::clock(59.2f) == "01:00");     // rounds up: never shows 0 early
    CHECK(Hud::clock(75.0f) == "01:15");
    CHECK(Hud::clock(-3.0f) == "00:00");
}

void unitConversions() {
    // 100 px/s * 0.2 m/px = 20 m/s = 72 km/h
    CHECK(Hud::kilometresPerHour(100.0f) == 72);
    CHECK(Hud::kilometresPerHour(-100.0f) == 72);
    CHECK(Hud::metres(500.0f) == 100);
}

}

int main() {
    segmentMasks();
    segmentCountsMatchMasks();
    clockFormatting();
    unitConversions();
    return check::failures();
}
