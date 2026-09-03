/**
 * ui.c — THE INPUT LAYER.  ****  YOU WRITE THE BOTTOM HALF  ****
 *
 * Every action here does the same three things and nothing else:
 *
 *      ask the user  ->  validate  ->  write ONE field of the model
 *
 * None of them decide anything. Deciding what a lamp should do is the job of
 * applyRules() in house.c. Keeping those apart is what makes either of them
 * testable — and it is worth marks.
 *
 *  THE FIVE FUNCTIONS ARE IN THE ORDER YOU SHOULD WRITE THEM.
 *  Work straight down the file. Each one is marked [ N / 5 ].
 *
 *      [ 1 / 5 ]  setOccupancy()     FR-08   easiest — start here
 *      [ 2 / 5 ]  setTemperature()   FR-09   ask, range-check, store
 *      [ 3 / 5 ]  switchDevice()     FR-07   a switch with 3 cases
 *      [ 4 / 5 ]  houseReport()      FR-11   counters and bars
 *      [ 5 / 5 ]  runAutomation()    FR-10   the trace — hardest, do it last
 *
 * The top half is GIVEN: the menu, the input helpers, and pickRoom(), which
 * is your validation template. Copy its shape.
 *
 * Smart Home Console · Day 03 midterm — G9
 * Student: <YOUR NAME HERE>
 */
#include <stdio.h>

#include "ui.h"
#include "house.h"
#include "render.h"
    #include "platform.h"

    void printMenu(void)
    {
        printf("\n  %s1%s) %-26s%s2%s) %s\n", CC(C_SEL), CC(C_RESET),
               "Show the house", CC(C_SEL), CC(C_RESET), "Switch lamp/fan/auto");
        printf("  %s3%s) %-26s%s4%s) %s\n", CC(C_SEL), CC(C_RESET),
               "Someone enters/leaves", CC(C_SEL), CC(C_RESET), "Set room temperature");
        printf("  %s5%s) %-26s%s6%s) %s\n", CC(C_SEL), CC(C_RESET),
               "Run automation", CC(C_SEL), CC(C_RESET), "House report");
        printf("  %s7%s) %-26s%s0%s) %s\n", CC(C_SEL), CC(C_RESET),
               "Auto demo (scripted)", CC(C_SEL), CC(C_RESET), "Exit");
        printf("\n  Select > ");
        fflush(stdout);
    }

    int readInt(int *out)
    {
        char buffer[64];
        if (fgets(buffer, (int)sizeof buffer, stdin) == NULL) { return 0; }
        return sscanf(buffer, "%d", out) == 1;
    }

    void pauseKey(void)
    {
        char buffer[64];
        if (g_plain) { return; }
        printf("\n%s  -- press Enter --%s", CC(C_DIM), CC(C_RESET));
        fflush(stdout);
        if (fgets(buffer, (int)sizeof buffer, stdin) == NULL) { }
    }

    void printBinary(uint8_t value)
    {
        printf("0b");
        for (int8_t bit = 7; bit >= 0; bit--) {
            putchar(READ_BIT(value, bit) ? '1' : '0');
        }
    }

    uint8_t pickRoom(void)
    {
        int n = -1;
        printf("  Room (");
        for (uint8_t i = 0U; i < ROOM_COUNT; i++) {
            printf("%s%u%s=%s ", CC(C_SEL), i, CC(C_RESET), houseRoom(i)->name);
        }
        printf("): ");
        if (!readInt(&n) || n < 0 || n >= (int)ROOM_COUNT) {
            statusSet(C_ALARM, "No such room.");
            return 255U;
        }
        return (uint8_t)n;
    }

void setOccupancy(void)
{
    uint8_t i = pickRoom();
    if (i == 255U)
    {
        return;
    }

    printf("Occupancy (0=empty 1=occupied): ");
    int occupied = -1;
    if (!readInt(&occupied) || (occupied != 0 && occupied != 1))
    {
        statusSet(C_ALARM, "Invalid occupancy value");
        return;
    }

    if (occupied)
    {
        SET_BIT(houseRoom(i)->status, BIT_OCCUPIED);
        statusSet(C_OK, "%s: Occupied", houseRoom(i)->name);
    }
    else
    {
        CLR_BIT(houseRoom(i)->status, BIT_OCCUPIED);
        statusSet(C_OK, "%s: Empty", houseRoom(i)->name);
    }

    render((int)i);
    pauseKey();
}

/* ==========================================================================
 *  [ 2 / 5 ]   YOUR WORK HERE  —  setTemperature()                   FR-09
 * --------------------------------------------------------------------------
 *  REQUIRES : house.c [ 1 / 6 ] houseInit() and [ 2 / 6 ] tempC().
 *  GIVES    : menu 4 works — you can heat a room up and watch its bar grow.
 *             This is how you will test your rules later.
 *  USES     : pickRoom(), readInt(), ADC_MAX, tempC(), statusSet(),
 *             render(), pauseKey()
 *  CHECK    : 1023 is accepted (499 C, bar full). 2000 and "abc" are both
 *             rejected AND the room's old temperature is unchanged.
 * ==========================================================================
 *
 * Write a new raw ADC count into one room.
 *
 *   1. i = pickRoom()
 *   2. ask "  Raw ADC reading (0..1023): " and read an int with readInt()
 *   3. reject anything outside 0..ADC_MAX with a statusSet() message and
 *      NO write to the model, then return
 *   4. otherwise store it and statusSet("%s: ADC %u -> %u C", ...)
 *   5. render((int)i), pauseKey()
 *
 * Read into an `int` and validate BEFORE casting to uint16_t. If you read
 * straight into a uint16_t you can never detect a negative number — it has
 * already wrapped to 65532 and sailed through your range check.
 */
void setTemperature(void)
{
    uint8_t i = pickRoom();
    if (i == 255)
    {
        return;
    }

    printf("Raw ADC reading (0..1023): ");
    int val = -1;
    if (!readInt(&val))
    {
        statusSet(C_ALARM, "Invalid ADC value");
        return;
    }

    if (val < 0 || val > ADC_MAX)
    {
        statusSet(C_ALARM, "Invalid ADC value");
        return;
    }

    uint16_t adc = (uint16_t)val;
    houseRoom(i)->adc = adc;
    uint16_t degC = tempC(adc);

    statusSet(C_OK, "%s: ADC %u -> %u C", houseRoom(i)->name, adc, degC);

    render((int)i);
    pauseKey();
}



/* ==========================================================================
 *  [ 3 / 5 ]   YOUR WORK HERE  —  switchDevice()                     FR-07
 * --------------------------------------------------------------------------
 *  REQUIRES : house.c [ 1 / 6 ] houseInit().
 *  GIVES    : menu 2 works — lamps and fans flip by hand, and the room drops
 *             to MAN. Once your rules exist, MANUAL rooms get skipped.
 *  USES     : pickRoom(), readInt(), TOGGLE_BIT, CLR_BIT, READ_BIT,
 *             BIT_LAMP, BIT_FAN, BIT_AUTO, statusSet(), render(),
 *             printBinary(), pauseKey()
 *  CHECK    : flip the Living lamp — the card shows [#] and the tag turns to
 *             MAN. The printed status byte must be 0b00000101 (LAMP + people,
 *             AUTO gone).
 * ==========================================================================
 *
 * Switch a lamp, a fan, or auto-mode.
 *
 *   1. i = pickRoom(); if 255, return
 *   2. ask "  Switch (1=Lamp 2=Fan 3=Auto mode): " and read an int
 *   3. switch on the answer:
 *        1 -> TOGGLE_BIT the LAMP, then CLR_BIT the AUTO bit
 *        2 -> TOGGLE_BIT the FAN,  then CLR_BIT the AUTO bit
 *        3 -> TOGGLE_BIT the AUTO bit
 *        anything else -> statusSet "Nothing switched." and return
 *   4. statusSet() a message saying what happened
 *   5. render((int)i), then print "  <name> status = " + printBinary(status)
 *      + "  (0x%02X)", then pauseKey()
 *
 * WHY LAMP AND FAN CLEAR AUTO: a human just touched the switch by hand, so
 * the room drops out of automation until somebody hands control back. That
 * is exactly how a real thermostat behaves — manual override wins.
 */
void switchDevice(void)
{
    uint8_t i = pickRoom();
    if (i == 255)
    {
        return;
    }

    printf("Switch (1-Lamp 2-Fan 3-Auto mode): ");
    int choice = 0;
    if (!readInt(&choice))
    {
        statusSet(C_ALARM, "Nothing switched.");
        return;
    }

    switch (choice)
    {
        case 1:
            TOGGLE_BIT(houseRoom(i)->status, BIT_LAMP);
            CLR_BIT(houseRoom(i)->status, BIT_AUTO);
            statusSet(C_LAMP, "%s: Lamp switched", houseRoom(i)->name);
            break;

        case 2:
            TOGGLE_BIT(houseRoom(i)->status, BIT_FAN);
            CLR_BIT(houseRoom(i)->status, BIT_AUTO);
            statusSet(C_FAN, "%s: Fan switched", houseRoom(i)->name);
            break;

        case 3:
            TOGGLE_BIT(houseRoom(i)->status, BIT_AUTO);
            statusSet(C_AUTO, "%s: Auto mode toggled", houseRoom(i)->name);
            break;

        default:
            statusSet(C_ALARM, "Nothing switched.");
            return;
    }

    render((int)i);
    printf("%s status = ", houseRoom(i)->name);
    printBinary(houseRoom(i)->status);
    printf(" (0x%02X)\n", houseRoom(i)->status);
    pauseKey();
}


/* ==========================================================================
 *  [ 4 / 5 ]   YOUR WORK HERE  —  houseReport()                      FR-11
 * --------------------------------------------------------------------------
 *  REQUIRES : house.c [ 5 / 6 ] countRoomsWith(), [ 6 / 6 ] sumAdc(),
 *             and [ 2 / 6 ] tempC().
 *  GIVES    : menu 6 works — the summary with its four bars.
 *  USES     : render(), countRoomsWith(), drawBar(), REPORT_BAR_W,
 *             sumAdc(), houseRooms(), tempC(), pauseKey()
 *  CHECK    : the counters must match what you can count on the schematic.
 *             With the seed house the raw sum is 363 and the average 29 C.
 * ==========================================================================
 *
 * The house report:
 *
 *   render(-1) first, so the schematic sits above your report
 *
 *   four counter lines, each  countRoomsWith(BIT_x)  followed by
 *       drawBar(count, ROOM_COUNT, REPORT_BAR_W, COLOUR)
 *     -> Lamps ON, Fans ON, Occupied, Alarms
 *
 *   hottest and coldest room BY NAME (one loop comparing .adc)
 *   average temperature: tempC(sumAdc(houseRooms(), ROOM_COUNT) / ROOM_COUNT)
 *
 *   then pauseKey()
 *
 * Use the SAME drawBar() that draws the temperature gauge in each room card.
 * One function, two scales. Four copy-pasted loops score less.
 */
void houseReport(void)
{
    render(-1);

    uint8_t count;

    count = countRoomsWith(BIT_LAMP);
    printf("Lamps ON: %u/%u ", count, ROOM_COUNT);
    drawBar(count, ROOM_COUNT, REPORT_BAR_W, C_LAMP);
    printf("\n");

    count = countRoomsWith(BIT_FAN);
    printf("Fans ON:  %u/%u ", count, ROOM_COUNT);
    drawBar(count, ROOM_COUNT, REPORT_BAR_W, C_FAN);
    printf("\n");

    count = countRoomsWith(BIT_OCCUPIED);
    printf("Occupied: %u/%u ", count, ROOM_COUNT);
    drawBar(count, ROOM_COUNT, REPORT_BAR_W, C_OK);
    printf("\n");

    count = countRoomsWith(BIT_ALARM);
    printf("Alarms:   %u/%u ", count, ROOM_COUNT);
    drawBar(count, ROOM_COUNT, REPORT_BAR_W, C_ALARM);
    printf("\n\n");

    const Room_t *rooms = houseRooms();
    uint8_t hottestIndex = 0U;
    uint8_t coldestIndex = 0U;

    for (uint8_t i = 1U; i < ROOM_COUNT; i++)
    {
        if (rooms[i].adc > rooms[hottestIndex].adc)
        {
            hottestIndex = i;
        }
        if (rooms[i].adc < rooms[coldestIndex].adc)
        {
            coldestIndex = i;
        }
    }

    printf("Hottest room: %s (%u C)\n", rooms[hottestIndex].name,
           tempC(rooms[hottestIndex].adc));
    printf("Coldest room: %s (%u C)\n", rooms[coldestIndex].name,
           tempC(rooms[coldestIndex].adc));

    uint16_t averageAdc = (uint16_t)(sumAdc(rooms, ROOM_COUNT) / ROOM_COUNT);
    printf("Average temp: %u C\n", tempC(averageAdc));

    pauseKey();
}


/* ==========================================================================
 *  [ 5 / 5 ]   YOUR WORK HERE  —  runAutomation()                    FR-10
 *              *** hardest one — leave it until last ***
 * --------------------------------------------------------------------------
 *  REQUIRES : house.c [ 3 / 6 ] applyRules() and [ 4 / 6 ] rulesPass().
 *             Nothing here works until the rules do.
 *  GIVES    : menu 5 works — the before -> after trace, the thing that proves
 *             your rules are right.
 *  USES     : houseRoom(), tempC(), READ_BIT, BIT_AUTO, applyRules(),
 *             snprintf(), render(), statusSet(), pauseKey()
 *  CHECK    : press 5 twice. You MUST see 5 changed, then 0 changed.
 * ==========================================================================
 *
 * Run the rules over the house and show what moved.
 *
 * The deciding is already done for you — call applyRules() from house.c.
 * What belongs HERE is only the reporting:
 *
 *   for each room i:
 *       remember the status byte BEFORE
 *       if the room is not AUTO -> note "<name>  <t> C   skipped (MANUAL)"
 *       else -> call applyRules(), add its return to a `changed` counter,
 *               and note "<name>  <t> C   0b<before> -> 0b<after>  *"
 *                (the * only when it actually changed)
 *   print every note, then "N room(s) changed.", then pauseKey()
 *
 * Build the notes into a `char trace[ROOM_COUNT][96];` with snprintf() so you
 * can print them all together at the end.
 *
 * SELF-CHECK — this is the one people fail. Run it twice. If a second pass
 * keeps reporting changes, a rule is fighting itself: go back to house.c
 * [ 3 / 6 ] and find the `if` that has no `else`.
 */
void runAutomation(void)
{
    char trace[ROOM_COUNT][96];
    uint8_t changedCount = 0U;

    render(-1);

    for (uint8_t i = 0U; i < ROOM_COUNT; i++)
    {
        Room_t *room = houseRoom(i);
        uint8_t before = room->status;
        uint16_t temperature = tempC(room->adc);

        if (!READ_BIT(before, BIT_AUTO))
        {
            snprintf(trace[i], sizeof(trace[i]), "%s %u C skipped (MANUAL)",
                     room->name, temperature);
        }
        else
        {
            uint8_t changed = applyRules(room);
            changedCount += changed;
            uint8_t after = room->status;
            char beforeBits[9];
            char afterBits[9];

            for (int bit = 7; bit >= 0; bit--)
            {
                beforeBits[7 - bit] = READ_BIT(before, bit) ? '1' : '0';
                afterBits[7 - bit] = READ_BIT(after, bit) ? '1' : '0';
            }
            beforeBits[8] = '\0';
            afterBits[8] = '\0';

            snprintf(trace[i], sizeof(trace[i]), "%s %u C   0b%s -> 0b%s %s",
                     room->name, temperature, beforeBits, afterBits,
                     changed ? "*" : "");
        }
    }
    for (uint8_t i = 0U; i < ROOM_COUNT; i++)
    {
        printf("%s\n", trace[i]);
    }
    printf("%u room(s) changed.\n", changedCount);

    pauseKey();
}

