#include "global.h"
#include "battle_anim.h"
#include "berry.h"
#include "berry_powder.h"
#include "bg.h"
#include "decompress.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item_icon.h"
#include "item_menu.h"
#include "link.h"
#include "link_rfu.h"
#include "main.h"
#include "malloc.h"
#include "math_util.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "minigame_countdown.h"
#include "random.h"
#include "digit_obj_util.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trig.h"
#include "window.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

struct BerryCrushGame_Player
{
    u8 unk0[PLAYER_NAME_LENGTH + 1 + 4];
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u16 unk12;
    u16 unk14;
    u16 unk16;
    u16 unk18;
    u16 unk1A;
    u8 unk1B;
    u8 unk1C;
};

struct BerryCrushGame_4E
{
    u16 unk0;
    u16 unk2;
    u8 unk4_0:1;
    u8 unk4_1:1;
    u8 unk4_2:1;
    u8 unk4_3:5;
    s8 unk5;
    u16 unk6;
    u16 unk8;
    u16 unkA;
    u16 unkC;
};

struct BerryCrushGame_40
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
};

struct BerryCrushGame_5C
{
    u16 unk00;
    u8 unk02_0:1;
    u8 unk02_1:1;
    u8 pushedAButton:1;
    u8 unk02_3:5;
    s8 unk03;
    u16 unk04;
    u16 unk06;
    u16 unk08;
    u16 unk0A;
};

struct BerryCrushGame_68
{
    u32 unk00;
    u16 unk04;
    u16 unk06;
    u16 unk08;
    u16 unk0A;
    // 0: Number of A presses
    // 1: Neatness
    u16 stats[2][5];
    u8 unk20[2][8];
};

struct BerryCrushPlayerSeatCoords
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
};

struct BerryCrushGame_138
{
    u8 animBerryIdx;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    s16 minutes;
    s16 secondsInt;
    s16 secondsFrac;
    const struct BerryCrushPlayerSeatCoords *seatCoords[5];
    struct Sprite *coreSprite;
    struct Sprite *impactSprites[5];
    struct Sprite *berrySprites[5];
    struct Sprite *sparkleSprites[11];
    struct Sprite *timerSprites[2];
    u8 unk80;
    u8 filler81;
    u8 unk82;
    u8 unk83[5];
};

struct BerryCrushGame
{
    MainCallback savedCallback;
    u32 (*cmdCallback)(struct BerryCrushGame *, u8 *);
    u8 localId;
    u8 playerCount;
    u8 mainTask;
    u8 textSpeed;
    u8 cmdState;
    u8 unkD;
    u8 nextCmd;
    u8 afterPalFadeCmd;
    u16 unk10;
    u16 gameState;
    u16 unk14;
    u16 pressingSpeed;
    s16 unk18;
    s16 unk1A;
    s32 powder;
    s32 unk20;
    u8 unk24;
    u8 unk25_0:1;
    u8 unk25_1:1;
    u8 unk25_2:1;
    u8 unk25_3:1;
    u8 unk25_4:1;
    u8 unk25_5:3;
    u16 unk26;
    u16 timer;
    s16 depth;
    s16 vibration;
    s16 unk2E;
    s16 unk30;
    s16 unk32;
    s16 unk34;
    u8 unk36[0xC];
    u16 unk42[6];
    u16 recvCmd[7];
    struct BerryCrushGame_5C localState;
    struct BerryCrushGame_68 unk68;
    struct BerryCrushGame_Player unk98[5];
    struct BerryCrushGame_138 unk138;
    u8 bg1Buffer[0x1000];
    u8 unk11C0[0x1000];
    u8 bg2Buffer[0x1000];
    u8 bg3Buffer[0x1000];
};

static void VBlankCB(void);
static void MainCB(void);
static void MainTask(u8);
static void ParseName_Options(struct BerryCrushGame *);
static void BerryCrush_RunOrScheduleCommand(u16, u8, u8 *);
static void BerryCrush_SetPaletteFadeParams(u8 *, bool8, u32, s8, u8, u8, u16);
static s32 sub_8021450(struct BerryCrushGame *);
static void sub_8022588(struct BerryCrushGame *);
static void sub_8022600(struct BerryCrushGame *);
static void sub_80226D0(struct BerryCrushGame *);
static void sub_8022730(struct BerryCrushGame *);
static void sub_8022960(struct BerryCrushGame *);
static void BerryCrush_PrintTimeOnSprites(struct BerryCrushGame_138 *, u16);
static void sub_8022B28(struct Sprite *);
static void BerryCrush_HideTimerSprites(struct BerryCrushGame_138 *r0);
static void sub_8024578(struct BerryCrushGame *);
static void BerryCrush_SetShowMessageParams(u8 *params, u8 stringId, u8 flags, u16 waitKeys, u8 followupCmd);
static void SpriteCB_BerryCrushImpact(struct Sprite *sprite);
static u32 BerryCrushCommand_BeginNormalPaletteFade(struct BerryCrushGame *r6, u8 *r1);
static u32 BerryCrushCommand_WaitPaletteFade(struct BerryCrushGame *r4, u8 *r5);
static u32 BerryCrushCommand_PrintMessage(struct BerryCrushGame *r7, u8 *r5);
static u32 BerryCrushCommand_InitGfx(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_TeardownGfx(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_SignalReadyToBegin(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_AskPickBerry(struct BerryCrushGame *r4, u8 *r5);
static u32 BerryCrushCommand_GoToBerryPouch(struct BerryCrushGame *r0, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_WaitForOthersToPickBerries(struct BerryCrushGame *r5, u8 *r2);
static u32 BerryCrushCommand_DropBerriesIntoCrusher(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_DropLid(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_Countdown(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_PlayGame_Master(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_PlayGame_Slave(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_FinishGame(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_HandleTimeUp(struct BerryCrushGame *r5, u8 *r6);
static u32 BerryCrushCommand_TabulateResults(struct BerryCrushGame *r7, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_ShowResults(struct BerryCrushGame *r5, u8 *r6);
static u32 BerryCrushCommand_SaveGame(struct BerryCrushGame *r5, u8 *r4);
static u32 BerryCrushCommand_AskPlayAgain(struct BerryCrushGame *r5, u8 *r6);
static u32 BerryCrushCommand_CommunicatePlayAgainResponses(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_FadeOutToPlayAgain(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_PlayAgainFailureMessage(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_GracefulExit(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1);
static u32 BerryCrushCommand_Quit(__attribute__((unused)) struct BerryCrushGame *r0, __attribute__((unused)) u8 *r1);

static EWRAM_DATA struct BerryCrushGame *sBerryCrushGamePtr = NULL;

static const u8 gUnknown_082F325C[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
static const u8 gUnknown_082F3264[] = { 0, 1, 2, 3, 5, 0, 0, 0 };

static const s8 gUnknown_082F326C[][7] = 
{
    { 4, 1, 0, -1,  0,  0, 0}, 
    { 4, 2, 0, -1,  0,  0, 0}, 
    { 4, 2, 0, -2,  0,  0, 0}, 
    { 6, 3, 1, -1, -3, -1, 0}, 
    { 6, 4, 1, -2, -4, -2, 0},
};

static const u8 sUnusedZero = 0;

static const u8 gUnknown_082F3290[][4] = 
{
    {3, 2, 1, 0}, 
    {3, 3, 1, 0},
    {3, 3, 2, 0},
    {3, 4, 2, 0},
    {3, 5, 3, 0},
};

static const u8 *const sBerryCrushMessages[] =
{
    gText_ReadyToBerryCrush,
    gText_WaitForAllChooseBerry,
    gText_EndedWithXUnitsPowder,
    gText_RecordingGameResults,
    gText_PlayBerryCrushAgain,
    gText_YouHaveNoBerries,
    gText_MemberDroppedOut,
    gText_TimesUpNoGoodPowder,
    gText_CommunicationStandby2,
};

static const struct BgTemplate gUnknown_082F32C8[4] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 2,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0, 
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0, 
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 11,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0, 
    },
};


static const u8 sBerryCrushTextColorTable[][3] =
{
    {TEXT_COLOR_WHITE,       TEXT_COLOR_DARK_GREY,  TEXT_COLOR_LIGHT_GREY},
    {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE,      TEXT_COLOR_DARK_GREY},
    {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GREY, TEXT_COLOR_RED},
    {TEXT_COLOR_WHITE,       TEXT_COLOR_BLUE,       TEXT_COLOR_LIGHT_BLUE},
    {TEXT_COLOR_WHITE,       TEXT_COLOR_GREEN,      TEXT_COLOR_LIGHT_GREEN},
    {TEXT_COLOR_WHITE,       TEXT_COLOR_RED,        TEXT_COLOR_LIGHT_RED},
};


static const struct WindowTemplate sWindowTemplate_BerryCrushRankings =
{
    .bg = 0, 
    .tilemapLeft = 3, 
    .tilemapTop = 4, 
    .width = 24, 
    .height = 13, 
    .paletteNum = 15,
    .baseBlock = 1
};

static const struct WindowTemplate gUnknown_082F32F4[] =
{
    {
        .bg = 0, 
        .tilemapLeft = 0, 
        .tilemapTop = 0, 
        .width = 9, 
        .height = 2, 
        .paletteNum = 8, 
        .baseBlock = 1005
    },
    {
        .bg = 0, 
        .tilemapLeft = 0, 
        .tilemapTop = 3, 
        .width = 9, 
        .height = 2, 
        .paletteNum = 8, 
        .baseBlock = 987
    },
    {
        .bg = 0, 
        .tilemapLeft = 0, 
        .tilemapTop = 6, 
        .width = 9, 
        .height = 2, 
        .paletteNum = 8, 
        .baseBlock = 969
    },
    {
        .bg = 0, 
        .tilemapLeft = 21, 
        .tilemapTop = 3, 
        .width = 9, 
        .height = 2, 
        .paletteNum = 8, 
        .baseBlock = 951
    },
    {
        .bg = 0, 
        .tilemapLeft = 21, 
        .tilemapTop = 6, 
        .width = 9, 
        .height = 2, 
        .paletteNum = 8, 
        .baseBlock = 933
    },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate gUnknown_082F3324[] =
{
    {
        .bg = 0, 
        .tilemapLeft = 5, 
        .tilemapTop = 2, 
        .width = 20, 
        .height = 16, 
        .paletteNum = 15, 
        .baseBlock = 1
    },
    {
        .bg = 0, 
        .tilemapLeft = 5, 
        .tilemapTop = 2, 
        .width = 20, 
        .height = 16, 
        .paletteNum = 15, 
        .baseBlock = 1
    },
    {
        .bg = 0, 
        .tilemapLeft = 4, 
        .tilemapTop = 2, 
        .width = 22, 
        .height = 16, 
        .paletteNum = 15, 
        .baseBlock = 1
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 gUnknown_082F3344[][4] =
{
    {6, 8, 9, 11}, 
    {12, 14, 15, 16},
};

static const u32 sPressingSpeedConversionTable[] = 
{
    // Decimal point is vertically aligned with the pixel
    // directly between the >< below.
    50000000, // 50
    25000000, // 25
    12500000, // 12.5
     6250000, //  6.25
     3125000, //  3.125
     1562500, //  1.5625
      781250, //  0.78125
      390625  //  0.390625
};

static const u16 gBerryCrushGrinderBasePal[] = INCBIN_U16("graphics/link_games/berrycrush_grinder_base.gbapal");
static const u16 gBerryCrushMiscSpritesPal[] = INCBIN_U16("graphics/link_games/berrycrush_misc.gbapal");
static const u16 gBerryCrushTimerDigitsPal[] = INCBIN_U16("graphics/link_games/berrycrush_timerdigits.gbapal");
static const u32 gBerryCrushGrinderBaseGfx[] = INCBIN_U32("graphics/link_games/berrycrush_grinder_base.4bpp.lz");
static const u32 gBerryCrushBtnPressGfx[] = INCBIN_U32("graphics/link_games/berrycrush_btnpress.4bpp.lz");
static const u32 gBerryCrushSparkleGfx[] = INCBIN_U32("graphics/link_games/berrycrush_sparkle.4bpp.lz");
static const u8 gBerryCrushTimerDigitsGfx[] = INCBIN_U8("graphics/link_games/berrycrush_timerdigits.4bpp.lz");
static const u8 gBerryCrushGrinderTopTilemap[] = INCBIN_U8("graphics/link_games/berrycrush_grinder_top.bin.lz");
static const u8 gBerryCrushContainerCapTilemap[] = INCBIN_U8("graphics/link_games/berrycrush_container_cap.bin.lz");
static const u8 gBerryCrushBackgroundTilemap[] = INCBIN_U8("graphics/link_games/berrycrush_background.bin.lz");

static const u8 gUnknown_082F417C[][5] = 
{
    {1, 3, 0, 0, 0}, 
    {0, 1, 3, 0, 0}, 
    {1, 3, 2, 4, 0},
    {0, 1, 3, 2, 4},
};

static const struct BerryCrushPlayerSeatCoords gUnknown_082F4190[] =
{
    {
        .unk0 = 0,
        .unk1 = 0,
        .unk2 = 0,
        .unk4 = 0,
        .unk6 = -16,
        .unk8 = 0,
        .unkA = 0,
    },
    {
        .unk0 = 1,
        .unk1 = 0,
        .unk2 = 3,
        .unk4 = -28,
        .unk6 = -4,
        .unk8 = -24,
        .unkA = 16,
    },
    {
        .unk0 = 2,
        .unk1 = 0,
        .unk2 = 6,
        .unk4 = -16,
        .unk6 = 20,
        .unk8 = -8,
        .unkA = 16,
    },
    {
        .unk0 = 3,
        .unk1 = 20,
        .unk2 = 3,
        .unk4 = 28,
        .unk6 = -4,
        .unk8 = 32,
        .unkA = -8,
    },
    {
        .unk0 = 4,
        .unk1 = 20,
        .unk2 = 6,
        .unk4 = 16,
        .unk6 = 20,
        .unk8 = 16,
        .unkA = -8,
    }
};


static const s8 gUnknown_082F41CC[][2] =
{
    { 0, 0},
    {-1, 0},
    { 1, 1},
};

static const s8 gUnknown_082F41D2[][2] =
{
    {  0,   0},
    {-16,  -4},
    { 16,  -4},
    { -8,  -2},
    {  8,  -2},
    {-24,  -8},
    { 24,  -8},
    {-32, -12},
    { 32, -12},
    {-40, -16},
    { 40, -16},
};

static const u16 sPlayerBerrySpriteTags[] = {5, 6, 7, 8, 9, 0};

static const struct CompressedSpriteSheet gUnknown_082F41F4[] = 
{
    { .data = gBerryCrushGrinderBaseGfx, .size = 0x800, .tag = 1 },
    { .data = gBerryCrushBtnPressGfx,    .size = 0xE00, .tag = 2 },
    { .data = gBerryCrushSparkleGfx,     .size = 0x700, .tag = 3 },
};

static const struct SpriteSheet gUnknown_082F420C[] =
{
    { .data = gBerryCrushTimerDigitsGfx, .size = 0x2C0, .tag = 4 },
    {}
};


static const struct SpritePalette sSpritePals[] =
{
    { .data = gBerryCrushGrinderBasePal, .tag = 1 },
    { .data = gBerryCrushMiscSpritesPal, .tag = 2 },
    { .data = gBerryCrushTimerDigitsPal, .tag = 4 },
    {}
};

static const union AnimCmd gUnknown_082F423C[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd gUnknown_082F4244[] =
{
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(16, 4),
    ANIMCMD_FRAME(32, 4),
    ANIMCMD_END
};

static const union AnimCmd gUnknown_082F4254[] =
{
    ANIMCMD_FRAME(48, 2),
    ANIMCMD_FRAME(64, 2),
    ANIMCMD_FRAME(80, 2),
    ANIMCMD_FRAME(96, 2),
    ANIMCMD_END
};

static const union AnimCmd gUnknown_082F4268[] =
{
    ANIMCMD_FRAME(0, 2),
    ANIMCMD_FRAME(4, 2), 
    ANIMCMD_FRAME(8, 2),
    ANIMCMD_FRAME(12, 2),
    ANIMCMD_FRAME(16, 2), 
    ANIMCMD_FRAME(20, 2), 
    ANIMCMD_JUMP(0)
};

static const union AnimCmd gUnknown_082F4284[] =
{
    ANIMCMD_FRAME(24, 4), 
    ANIMCMD_FRAME(28, 4), 
    ANIMCMD_FRAME(32, 4), 
    ANIMCMD_FRAME(36, 4),
    ANIMCMD_FRAME(40, 4), 
    ANIMCMD_FRAME(44, 4), 
    ANIMCMD_FRAME(48, 4), 
    ANIMCMD_FRAME(52, 4),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd gUnknown_082F42A8[] =
{
    ANIMCMD_FRAME(20, 0),
    ANIMCMD_END
};

static const union AnimCmd gUnknown_082F42B0[] =
{
    ANIMCMD_FRAME(0, 0), 
    ANIMCMD_END
};


static const union AffineAnimCmd gUnknown_082F42B8[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_FRAME(0, 0, 2, 1),
    AFFINEANIMCMD_JUMP(1)
};

static const union AffineAnimCmd gUnknown_082F42D0[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMD_FRAME(0, 0, -2, 1),
    AFFINEANIMCMD_JUMP(1)
};

static const union AnimCmd *const sAnimTable_BerryCrushCore[] =
{
    gUnknown_082F423C
};

static const union AnimCmd *const sAnimTable_BerryCrushImpact[] =
{
    gUnknown_082F4244,
    gUnknown_082F4254,
};

static const union AnimCmd *const sAnimTable_BerryCrushPowderSparkles[] =
{
    gUnknown_082F4268,
    gUnknown_082F4284,
};

static const union AnimCmd *const sAnimTable_BerryCrushTimer[] =
{
    gUnknown_082F42A8
};

static const union AnimCmd *const gUnknown_082F4300[] =
{
    gUnknown_082F42B0
};

static const union AffineAnimCmd *const gUnknown_082F4304[] =
{
    gUnknown_082F42B8,
    gUnknown_082F42D0,
};

static const struct SpriteTemplate sSpriteTemplate_BerryCrushCore =
{
    .tileTag = 1, 
    .paletteTag = 1, 
    .oam = &gOamData_AffineOff_ObjNormal_64x64, 
    .anims = sAnimTable_BerryCrushCore, 
    .images = NULL, 
    .affineAnims = gDummySpriteAffineAnimTable, 
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sSpriteTemplate_BerryCrushImpact =
{
    .tileTag = 2, 
    .paletteTag = 2, 
    .oam = &gOamData_AffineOff_ObjNormal_32x32, 
    .anims = sAnimTable_BerryCrushImpact, 
    .images = NULL, 
    .affineAnims = gDummySpriteAffineAnimTable, 
    .callback = SpriteCB_BerryCrushImpact
};

static const struct SpriteTemplate sSpriteTemplate_BerryCrushPowderSparkles =
{
    .tileTag = 3, 
    .paletteTag = 2, 
    .oam = &gOamData_AffineOff_ObjNormal_16x16, 
    .anims = sAnimTable_BerryCrushPowderSparkles, 
    .images = NULL, 
    .affineAnims = gDummySpriteAffineAnimTable, 
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sSpriteTemplate_BerryCrushTimer =
{
    .tileTag = 4, 
    .paletteTag = 4, 
    .oam = &gOamData_AffineOff_ObjNormal_8x16, 
    .anims = sAnimTable_BerryCrushTimer, 
    .images = NULL, 
    .affineAnims = gDummySpriteAffineAnimTable, 
    .callback = SpriteCallbackDummy
};

static const struct SpriteTemplate sSpriteTemplate_PlayerBerry =
{
    .tileTag = 5, 
    .paletteTag = 5, 
    .oam = &gOamData_AffineDouble_ObjNormal_32x32, 
    .anims = gUnknown_082F4300, 
    .images = NULL, 
    .affineAnims = gUnknown_082F4304, 
    .callback = SpriteCallbackDummy
};

static const struct DigitObjUtilTemplate sDigitObjTemplates[] = 
{
    {
        .strConvMode = 1,
        .shape = 2,
        .size = 0,
        .priority = 0,
        .oamCount = 2, 
        .xDelta = 8, 
        .x = 156,
        .y = 0,
        .spriteSheet = gUnknown_082F420C,
        .spritePal = &sSpritePals[2],
    },
    {
        .strConvMode = 0,
        .shape = 2,
        .size = 0,
        .priority = 0,
        .oamCount = 2, 
        .xDelta = 8, 
        .x = 180,
        .y = 0,
        .spriteSheet = gUnknown_082F420C,
        .spritePal = &sSpritePals[2],
    },
    {
        .strConvMode = 0,
        .shape = 2,
        .size = 0,
        .priority = 0,
        .oamCount = 2, 
        .xDelta = 8, 
        .x = 204,
        .y = 0,
        .spriteSheet = gUnknown_082F420C,
        .spritePal = &sSpritePals[2],
    }
};

static const u8 *const sBCRankingHeaders[] =
{
    gText_SpaceTimes2,
    gText_XDotY,
    gText_Var1Berry,
    gText_NeatnessRankings,
    gText_CoopRankings,
    gText_PressingPowerRankings,
};

static u32 (*const sBerryCrushCommands[])(struct BerryCrushGame *, u8 *) =
{
    NULL,
    BerryCrushCommand_BeginNormalPaletteFade,
    BerryCrushCommand_WaitPaletteFade,
    BerryCrushCommand_PrintMessage,
    BerryCrushCommand_InitGfx,
    BerryCrushCommand_TeardownGfx,
    BerryCrushCommand_SignalReadyToBegin,
    BerryCrushCommand_AskPickBerry,
    BerryCrushCommand_GoToBerryPouch,
    BerryCrushCommand_WaitForOthersToPickBerries,
    BerryCrushCommand_DropBerriesIntoCrusher,
    BerryCrushCommand_DropLid,
    BerryCrushCommand_Countdown,
    BerryCrushCommand_PlayGame_Master,
    BerryCrushCommand_PlayGame_Slave,
    BerryCrushCommand_FinishGame,
    BerryCrushCommand_HandleTimeUp,
    BerryCrushCommand_TabulateResults,
    BerryCrushCommand_ShowResults,
    BerryCrushCommand_SaveGame,
    BerryCrushCommand_AskPlayAgain,
    BerryCrushCommand_CommunicatePlayAgainResponses,
    BerryCrushCommand_FadeOutToPlayAgain,
    BerryCrushCommand_PlayAgainFailureMessage,
    BerryCrushCommand_GracefulExit,
    BerryCrushCommand_Quit,
};

static const u8 gUnknown_082F4434[][4] =
{
    {2,  4,  6,  7}, 
    {3,  5,  8, 11},
    {3,  7, 11, 15},
    {4,  8, 12, 17},
};

static const u8 gUnknown_082F4444[] = {5, 7, 9, 12};
static const u8 sReceivedPlayerBitmasks[] = {0x03, 0x07, 0x0F, 0x1F};

struct BerryCrushGame * GetBerryCrushGame(void)
{
    return sBerryCrushGamePtr;
}

u32 QuitBerryCrush(MainCallback callback)
{
    if (!sBerryCrushGamePtr)
        return 2;

    if (!callback)
        callback = sBerryCrushGamePtr->savedCallback;

    DestroyTask(sBerryCrushGamePtr->mainTask);
    FREE_AND_SET_NULL(sBerryCrushGamePtr);
    SetMainCallback2(callback);
    if (callback == CB2_ReturnToField)
    {
        gTextFlags.autoScroll = TRUE;
        PlayNewMapMusic(MUS_POKE_CENTER);
        SetMainCallback1(CB1_Overworld);
    }

    return 0;
}

void StartBerryCrush(MainCallback callback)
{
    u8 playerCount = 0;
    u8 multiplayerId;

    if (!gReceivedRemoteLinkPlayers || gWirelessCommType == 0)
    {
        SetMainCallback2(callback);
        Rfu.unk_10 = 0;
        Rfu.unk_12 = 0;
        Rfu.errorState = 1;
        return;
    }

    playerCount = GetLinkPlayerCount();
    multiplayerId = GetMultiplayerId();
    if (playerCount < 2 || multiplayerId >= playerCount)
    {
        SetMainCallback2(callback);
        Rfu.unk_10 = 0;
        Rfu.unk_12 = 0;
        Rfu.errorState = 1;
        return;
    }

    sBerryCrushGamePtr = AllocZeroed(sizeof(struct BerryCrushGame));
    if (!sBerryCrushGamePtr)
    {
        SetMainCallback2(callback);
        Rfu.unk_10 = 0;
        Rfu.unk_12 = 0;
        Rfu.errorState = 1;
        return;
    }

    sBerryCrushGamePtr->savedCallback = callback;
    sBerryCrushGamePtr->localId = multiplayerId;
    sBerryCrushGamePtr->playerCount = playerCount;
    ParseName_Options(sBerryCrushGamePtr);
    sBerryCrushGamePtr->gameState = 1;
    sBerryCrushGamePtr->nextCmd = 1;
    sBerryCrushGamePtr->afterPalFadeCmd = 6;
    BerryCrush_SetPaletteFadeParams(sBerryCrushGamePtr->unk36, 1, -1, 0, 16, 0, 0);
    BerryCrush_RunOrScheduleCommand(4, 1, sBerryCrushGamePtr->unk36);
    SetMainCallback2(MainCB);
    sBerryCrushGamePtr->mainTask = CreateTask(MainTask, 8);
    gTextFlags.autoScroll = 0;
}

static void GetBerryFromBag(void)
{
    if (gSpecialVar_ItemId < FIRST_BERRY_INDEX || gSpecialVar_ItemId > LAST_BERRY_INDEX + 1)
        gSpecialVar_ItemId = FIRST_BERRY_INDEX;
    else
        RemoveBagItem(gSpecialVar_ItemId, 1);

    sBerryCrushGamePtr->unk98[sBerryCrushGamePtr->localId].unkC = gSpecialVar_ItemId - FIRST_BERRY_INDEX;
    sBerryCrushGamePtr->nextCmd = 1;
    sBerryCrushGamePtr->afterPalFadeCmd = 9;
    BerryCrush_SetPaletteFadeParams(sBerryCrushGamePtr->unk36, 0, -1, 0, 16, 0, 0);
    BerryCrush_RunOrScheduleCommand(4, 1, sBerryCrushGamePtr->unk36);
    sBerryCrushGamePtr->mainTask = CreateTask(MainTask, 8);
    SetMainCallback2(MainCB);
}

static void BerryCrush_SetupMainTask(void)
{
    DestroyTask(sBerryCrushGamePtr->mainTask);
    ChooseBerryForMachine(GetBerryFromBag);
}

static void BerryCrush_SetVBlankCB(void)
{
    SetVBlankCallback(VBlankCB);
}

static void BerryCrush_InitVBlankCB(void)
{
    SetVBlankCallback(NULL);
}

static void BerryCrush_SaveResults(void)
{
    u32 var0, var1;

    var0 = sBerryCrushGamePtr->unk68.unk04;
    var0 = Q_24_8(var0);
    var0 = MathUtil_Div32(var0, Q_24_8(60));
    var1 = sBerryCrushGamePtr->unk68.unk0A;
    var1 = Q_24_8(var1);
    var1 = MathUtil_Div32(var1, var0) & 0xFFFF;
    sBerryCrushGamePtr->pressingSpeed = var1;
    switch (sBerryCrushGamePtr->playerCount)
    {
    case 2:
        if (sBerryCrushGamePtr->pressingSpeed > gSaveBlock2Ptr->berryCrush.berryCrushResults[0])
        {
            sBerryCrushGamePtr->unk25_1 = 1;
            gSaveBlock2Ptr->berryCrush.berryCrushResults[0] = sBerryCrushGamePtr->pressingSpeed;
        }
        break;
    case 3:
        if (sBerryCrushGamePtr->pressingSpeed > gSaveBlock2Ptr->berryCrush.berryCrushResults[1])
        {
            sBerryCrushGamePtr->unk25_1 = 1;
            gSaveBlock2Ptr->berryCrush.berryCrushResults[1] = sBerryCrushGamePtr->pressingSpeed;
        }
        break;
    case 4:
        if (sBerryCrushGamePtr->pressingSpeed > gSaveBlock2Ptr->berryCrush.berryCrushResults[2])
        {
            sBerryCrushGamePtr->unk25_1 = 1;
            gSaveBlock2Ptr->berryCrush.berryCrushResults[2] = sBerryCrushGamePtr->pressingSpeed;
        }
        break;
    case 5:
        if (sBerryCrushGamePtr->pressingSpeed > gSaveBlock2Ptr->berryCrush.berryCrushResults[3])
        {
            sBerryCrushGamePtr->unk25_1 = 1;
            gSaveBlock2Ptr->berryCrush.berryCrushResults[3] = sBerryCrushGamePtr->pressingSpeed;
        }
        break;
    }

    sBerryCrushGamePtr->powder = sBerryCrushGamePtr->unk68.unk00;
    if (GiveBerryPowder(sBerryCrushGamePtr->powder))
        return;

    sBerryCrushGamePtr->unk25_0 = 1;
}

static void VBlankCB(void)
{
    TransferPlttBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}

static void MainCB(void)
{
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
}

static void MainTask(u8 taskId)
{
    if (sBerryCrushGamePtr->cmdCallback)
        sBerryCrushGamePtr->cmdCallback(sBerryCrushGamePtr, sBerryCrushGamePtr->unk36);

    sub_8021450(sBerryCrushGamePtr);
}

static void ParseName_Options(struct BerryCrushGame *arg0)
{
    u8 i = 0;

    for (; i < arg0->playerCount; i++)
        StringCopy(arg0->unk98[i].unk0, gLinkPlayers[i].name);
    for (; i < 5; i++)
    {
        memset(arg0->unk98[i].unk0, 1, PLAYER_NAME_LENGTH);
        arg0->unk98[i].unk0[PLAYER_NAME_LENGTH] = EOS;
    }

    switch (gSaveBlock2Ptr->optionsTextSpeed)
    {
    case OPTIONS_TEXT_SPEED_SLOW:
        arg0->textSpeed = 8;
        break;
    case OPTIONS_TEXT_SPEED_MID:
        arg0->textSpeed = 4;
        break;
    case OPTIONS_TEXT_SPEED_FAST:
        arg0->textSpeed = 1;
        break;
    }
}

// TODO: Everything from here on is likely in separate files.
s32 InitBerryCrushDisplay(void)
{
    struct BerryCrushGame *game = GetBerryCrushGame();
    if (!game)
        return -1;

    switch (game->cmdState)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ScanlineEffect_Stop();
        ResetTempTileDataBuffers();
        break;
    case 1:
        CpuFill16(0, (void *)OAM, OAM_SIZE);
        gReservedSpritePaletteCount = 0;
        DigitObjUtil_Init(3);
        break;
    case 2:
        ResetPaletteFade();
        ResetSpriteData();
        FreeAllSpritePalettes();
        break;
    case 3:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, gUnknown_082F32C8, ARRAY_COUNT(gUnknown_082F32C8));
        SetBgTilemapBuffer(1, game->bg1Buffer);
        SetBgTilemapBuffer(2, game->bg2Buffer);
        SetBgTilemapBuffer(3, game->bg3Buffer);
        ChangeBgX(0, 0, 0);
        ChangeBgY(0, 0, 0);
        ChangeBgX(2, 0, 0);
        ChangeBgY(2, 0, 0);
        ChangeBgX(3, 0, 0);
        ChangeBgY(3, 0, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        break;
    case 4:
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 64);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 32, 32);
        break;
    case 5:
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        DecompressAndCopyTileDataToVram(1, gUnknown_08DE34B8, 0, 0, 0);
        break;
    case 6:
        if (FreeTempTileDataBuffersIfPossible())
            return 0;

        InitStandardTextBoxWindows();
        InitTextBoxGfxAndPrinters();
        sub_8022588(game);
        sub_8022600(game);
        gPaletteFade.bufferTransferDisabled = TRUE;
        break;
    case 7:
        LoadPalette(gUnknown_08DE3398, 0, 0x180);
        CopyToBgTilemapBuffer(1, gBerryCrushGrinderTopTilemap, 0, 0);
        CopyToBgTilemapBuffer(2, gBerryCrushContainerCapTilemap, 0, 0);
        CopyToBgTilemapBuffer(3, gBerryCrushBackgroundTilemap, 0, 0);
        sub_80226D0(game);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        break;
    case 8:
        LoadWirelessStatusIndicatorSpriteGfx();
        CreateWirelessStatusIndicatorSprite(0,  0);
        sub_8022730(game);
        SetGpuReg(REG_OFFSET_BG1VOFS, -gSpriteCoordOffsetY);
        ChangeBgX(1, 0, 0);
        ChangeBgY(1, 0, 0);
        break;
    case 9:
        gPaletteFade.bufferTransferDisabled = FALSE;
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        BerryCrush_SetVBlankCB();
        game->cmdState = 0;
        return 1;
    }

    game->cmdState++;
    return 0;
}

static s32 BerryCrush_TeardownBgs(void)
{
    struct BerryCrushGame *var0 = GetBerryCrushGame();
    if (!var0)
        return -1;

    switch (var0->cmdState)
    {
    case 0:
        Rfu_SetLinkStandbyCallback();
        break;
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        // fall through. The original author forgot to use "break" here
        // because this will call BeginNormalPaletteFade() twice.
    case 2:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        UpdatePaletteFade();
        break;
    case 3:
        if (UpdatePaletteFade())
            return 0;
        break;
    case 4:
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 32, 32);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        break;
    case 5:
        FreeAllWindowBuffers();
        HideBg(0);
        UnsetBgTilemapBuffer(0);
        HideBg(1);
        UnsetBgTilemapBuffer(1);
        HideBg(2);
        UnsetBgTilemapBuffer(2);
        HideBg(3);
        UnsetBgTilemapBuffer(3);
        ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        break;
    case 6:
        DestroyWirelessStatusIndicatorSprite();
        sub_8022960(var0);
        DigitObjUtil_Free();
        break;
    case 7:
        var0->cmdState = 0;
        return 1;
    }

    var0->cmdState++;
    return 0;
}

static s32 sub_8021450(struct BerryCrushGame *arg0)
{
    gSpriteCoordOffsetY = arg0->depth + arg0->vibration;
    SetGpuReg(REG_OFFSET_BG1VOFS, -gSpriteCoordOffsetY);
    if (arg0->gameState == 7)
    {
        BerryCrush_PrintTimeOnSprites(&arg0->unk138, arg0->timer);
    }

    return 0;
}

void sub_8021488(struct BerryCrushGame *arg0)
{
    arg0->depth = -104;
    arg0->vibration = 0;
    gSpriteCoordOffsetX = 0;
    gSpriteCoordOffsetY = -104;
}

static void BerryCrush_CreateBerrySprites(struct BerryCrushGame *arg0, struct BerryCrushGame_138 *arg1)
{
    u8 i;
    u8 spriteId;
    s16 var0, var1;
    s16 *data;
    s32 var3;
    s16 var5;
    u32 var6;

    for (i = 0; i < arg0->playerCount; i++)
    {
        spriteId = AddCustomItemIconSprite(
            &sSpriteTemplate_PlayerBerry,
            sPlayerBerrySpriteTags[i],
            sPlayerBerrySpriteTags[i],
            arg0->unk98[i].unkC + FIRST_BERRY_INDEX);
        arg1->berrySprites[i] = &gSprites[spriteId];
        arg1->berrySprites[i]->oam.priority = 3;
        arg1->berrySprites[i]->affineAnimPaused = TRUE;
        arg1->berrySprites[i]->pos1.x = arg1->seatCoords[i]->unk8 + 120;
        arg1->berrySprites[i]->pos1.y = -16;
        data = arg1->berrySprites[i]->data;
        var5 = 512;
        data[1] = var5;
        data[2] = 32;
        data[7] = 112;
        var0 = arg1->seatCoords[i]->unkA - arg1->seatCoords[i]->unk8;
        var3 = var0;
        if (var0 < 0)
            var3 += 3;

        data[6] = var3 >> 2;
        var0 *= 128;
        var6 = var5 + 32;
        var6 = var6 / 2;
        var1 = MathUtil_Div16Shift(7, Q_8_8(63.5), var6);
        data[0] = (u16)arg1->berrySprites[i]->pos1.x * 128;
        data[3] = MathUtil_Div16Shift(7, var0, var1);
        var1 = MathUtil_Mul16Shift(7, var1, 85);
        data[4] = 0;
        data[5] = MathUtil_Div16Shift(7, Q_8_8(63.5), var1);
        data[7] |= 0x8000;
        if (arg1->seatCoords[i]->unk8 < 0)
            StartSpriteAffineAnim(arg1->berrySprites[i], 1);
    }
}

static void SpriteCB_DropBerryIntoCrusher(struct Sprite *sprite)
{
    s16 *data = sprite->data;

    data[1] += data[2];
    sprite->pos2.y += data[1] >> 8;
    if (data[7] & 0x8000)
    {
        sprite->data[0] += data[3];
        data[4] += data[5];
        sprite->pos2.x = Sin(data[4] >> 7, data[6]);
        if ((data[7] & 0x8000) && (data[4] >> 7) > 126)
        {
            sprite->pos2.x = 0;
            data[7] &= 0x7FFF;
        }
    }

    sprite->pos1.x = data[0] >> 7;
    if (sprite->pos1.y + sprite->pos2.y >= (data[7] & 0x7FFF))
    {
        sprite->callback = SpriteCallbackDummy;
        FreeSpriteOamMatrix(sprite);
        DestroySprite(sprite);
    }
}

void BerryCrushFreeBerrySpriteGfx(struct BerryCrushGame *arg0, __attribute__((unused)) struct BerryCrushGame_138 *arg1)
{
    u8 i;
    for (i = 0; i < arg0->playerCount; i++)
    {
        FreeSpritePaletteByTag(sPlayerBerrySpriteTags[i]);
        FreeSpriteTilesByTag(sPlayerBerrySpriteTags[i]);
    }
}

void sub_80216E0(struct BerryCrushGame *arg0, struct BerryCrushGame_138 *arg1)
{
    u8 sp4;
    struct BerryCrushGame_4E *var4E;
    u8 i;
    u16 var, var2;

    sp4 = 0;
    var4E = (struct BerryCrushGame_4E *)arg0->recvCmd;
    for (i = 0; i < arg0->playerCount; i++)
    {
        var = var4E->unkA >> (i * 3);
        var &= 7;
        if (var)
        {
            sp4++;
            if (var & 0x4)
                StartSpriteAnim(arg1->impactSprites[i], 1);
            else
                StartSpriteAnim(arg1->impactSprites[i], 0);

            arg1->impactSprites[i]->invisible = FALSE;
            arg1->impactSprites[i]->animPaused = FALSE;
            arg1->impactSprites[i]->pos2.x = gUnknown_082F41CC[(var % 4) - 1][0];
            arg1->impactSprites[i]->pos2.y = gUnknown_082F41CC[(var % 4) - 1][1];
        }
    }

    if (sp4 == 0)
    {
        arg0->unk25_2 = 0;
    }
    else
    {
        var = (u8)(arg0->timer % 3);
        var2 = var;
        for (i = 0; i < var4E->unkC * 2 + 3; i++)
        {
            if (arg1->sparkleSprites[i]->invisible)
            {
                arg1->sparkleSprites[i]->callback = sub_8022B28;
                arg1->sparkleSprites[i]->pos1.x = gUnknown_082F41D2[i][0] + 120;
                arg1->sparkleSprites[i]->pos1.y = gUnknown_082F41D2[i][1] + 136 - (var * 4);
                arg1->sparkleSprites[i]->pos2.x = gUnknown_082F41D2[i][0] + (gUnknown_082F41D2[i][0] / (var2 * 4));
                arg1->sparkleSprites[i]->pos2.y = gUnknown_082F41D2[i][1];
                if (var4E->unk4_1)
                    StartSpriteAnim(arg1->sparkleSprites[i], 1);
                else
                    StartSpriteAnim(arg1->sparkleSprites[i], 0);

                var++;
                if (var > 3)
                    var = 0;
            }
        }

        if (arg0->unk25_2)
        {
            arg0->unk25_2 = 0;
        }
        else
        {
            if (sp4 == 1)
                PlaySE(SE_MUD_BALL);
            else
                PlaySE(SE_BREAKABLE_DOOR);

            arg0->unk25_2 = 1;
        }
    }
}

bool32 sub_80218D4(struct BerryCrushGame *arg0, struct BerryCrushGame_138 *arg1)
{
    u8 i;

    for (i = 0; i < arg0->playerCount; i++)
    {
        if (!arg1->impactSprites[i]->invisible)
            return FALSE;
    }

    for (i = 0; i < 11; i++)
    {
        if (!arg1->sparkleSprites[i]->invisible)
            return FALSE;
    }

    if (arg0->vibration != 0)
        arg0->vibration = 0;

    return TRUE;
}

static void FramesToMinSec(struct BerryCrushGame_138 *arg0, u16 arg1)
{
    u8 i = 0;
    u32 fractionalFrames = 0;
    s16 r3 = 0;

    arg0->minutes = arg1 / 3600;
    arg0->secondsInt = (arg1 % 3600) / 60;
    r3 = MathUtil_Mul16(Q_8_8(arg1 % 60), 4);

    for (i = 0; i < 8; i++)
    {
        if ((r3 >> (7 - i)) & 1)
            fractionalFrames += sPressingSpeedConversionTable[i];
    }

    arg0->secondsFrac = fractionalFrames / 1000000;
}

static void PrintTextCentered(u8 windowId, u8 left, u8 colorId, const u8 *string)
{
    left = (left * 4) - (GetStringWidth(2, string, -1) / 2u);
    AddTextPrinterParameterized3(windowId, 2, left, 0, sBerryCrushTextColorTable[colorId], 0, string);
}

static void PrintBerryCrushResultWindow(struct BerryCrushGame * sp0C, u8 sp10, u8 sp14, u8 sp18)
{
    u8 r8;
    u8 sp1C = 0;
    u8 sp20 = 0;
    u8 r2;
    s32 r3;
    u8 r7;
    struct BerryCrushGame_68 * sp24 = &sp0C->unk68;
    u32 xOffset;
    s32 r6;

    sp18 -= 16;
    if (sp10 == 2)
        sp18 -= 42;
    r6 = sp18 - 14 * sp0C->playerCount;
    if (r6 > 0)
        r6 = r6 / 2 + 16;
    else
        r6 = 16;

    for (r8 = 0; r8 < sp0C->playerCount; r6 += 14, ++r8)
    {
        DynamicPlaceholderTextUtil_Reset();
        switch (sp10)
        {
        case 0:
            sp1C = sp24->unk20[sp10][r8];
            if (r8 != 0 && sp24->stats[sp10][r8] != sp24->stats[sp10][r8 - 1])
                sp20 = r8;
            ConvertIntToDecimalStringN(gStringVar4, sp24->stats[sp10][r8], STR_CONV_MODE_RIGHT_ALIGN, 4);
            StringAppend(gStringVar4, sBCRankingHeaders[sp10]);
            break;
        case 1:
            sp1C = sp24->unk20[sp10][r8];
            if (r8 != 0 && sp24->stats[sp10][r8] != sp24->stats[sp10][r8 - 1])
                sp20 = r8;
            ConvertIntToDecimalStringN(gStringVar1, sp24->stats[sp10][r8] >> 4, STR_CONV_MODE_RIGHT_ALIGN, 3);
            xOffset = 0;
            r7 = sp24->stats[sp10][r8] & 15;
            for (r2 = 0; r2 < 4; ++r2)
                if ((r7 >> (3 - r2)) & 1)
                    xOffset += sPressingSpeedConversionTable[r2];
            r7 = xOffset / 1000000u;
            ConvertIntToDecimalStringN(gStringVar2, r7, STR_CONV_MODE_LEADING_ZEROS, 2);
            StringExpandPlaceholders(gStringVar4, sBCRankingHeaders[sp10]);
            break;
        case 2:
            sp1C = r8;
            sp20 = r8;
            r2 = sp0C->unk98[r8].unkC;
            if (r2 >= LAST_BERRY_INDEX - FIRST_BERRY_INDEX + 2)
                r2 = 0;
            StringCopy(gStringVar1, gBerries[r2].name);
            StringExpandPlaceholders(gStringVar4, sBCRankingHeaders[sp10]);
            break;
        }
        r3 = GetStringRightAlignXOffset(2, gStringVar4, sp14 - 4);
        AddTextPrinterParameterized3(sp0C->unk138.unk82, 2, r3, r6, sBerryCrushTextColorTable[0], 0, gStringVar4);
        if (sp1C == sp0C->localId)
            StringCopy(gStringVar3, gText_1DotBlueF700);
        else
            StringCopy(gStringVar3, gText_1DotF700);
        gStringVar3[0] = sp20 + CHAR_1;
        DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, sp0C->unk98[sp1C].unk0);
        DynamicPlaceholderTextUtil_ExpandPlaceholders(gStringVar4, gStringVar3);
        AddTextPrinterParameterized3(sp0C->unk138.unk82, 2, 4, r6, sBerryCrushTextColorTable[0], 0, gStringVar4);
    }
}

static void sub_8021D34(struct BerryCrushGame *r8)
{
    u8 r10 = 0;
    u8 r6 = 0;
    u32 sp0C = 0;
    struct BerryCrushGame_68 *sp10 = &r8->unk68;
    u8 r7 = GetWindowAttribute(r8->unk138.unk82, WINDOW_HEIGHT) * 8 - 42;

    FramesToMinSec(&r8->unk138, sp10->unk04);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gText_TimeColon);
    r6 = 176 - (u8)GetStringWidth(2, gText_SpaceSec, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gText_SpaceSec);
    ConvertIntToDecimalStringN(gStringVar1, r8->unk138.secondsInt, STR_CONV_MODE_LEADING_ZEROS, 2);
    ConvertIntToDecimalStringN(gStringVar2, r8->unk138.secondsFrac, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringExpandPlaceholders(gStringVar4, gText_XDotY2);
    r6 -= GetStringWidth(2, gStringVar4, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gStringVar4);
    r6 -= GetStringWidth(2, gText_SpaceMin, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gText_SpaceMin);
    ConvertIntToDecimalStringN(gStringVar1, r8->unk138.minutes, STR_CONV_MODE_LEADING_ZEROS, 1);
    StringExpandPlaceholders(gStringVar4, gText_StrVar1);
    r6 -= GetStringWidth(2, gStringVar4, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gStringVar4);
    r7 += 14;
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, 0, r7, sBerryCrushTextColorTable[0], 0, gText_PressingSpeed);
    r6 = 176 - (u8)GetStringWidth(2, gText_TimesPerSec, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gText_TimesPerSec);
    for (; r10 < 8; ++r10)
        if (((u8)r8->pressingSpeed >> (7 - r10)) & 1)
            sp0C += *(r10 + sPressingSpeedConversionTable); // It's accessed in a different way here for unknown reason
    ConvertIntToDecimalStringN(gStringVar1, r8->pressingSpeed >> 8, STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, sp0C / 1000000, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringExpandPlaceholders(gStringVar4, gText_XDotY3);
    r6 -= GetStringWidth(2, gStringVar4, -1);
    if (r8->unk25_1)
        AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[5], 0, gStringVar4);
    else
        AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gStringVar4);
    r7 += 14;
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, 0, r7, sBerryCrushTextColorTable[0], 0, gText_Silkiness);
    ConvertIntToDecimalStringN(gStringVar1, sp10->unk08, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_Var1Percent);
    r6 = 176 - (u8)GetStringWidth(2, gStringVar4, -1);
    AddTextPrinterParameterized3(r8->unk138.unk82, 2, r6, r7, sBerryCrushTextColorTable[0], 0, gStringVar4);
}

static bool32 sub_8022070(struct BerryCrushGame *r4, struct BerryCrushGame_138 *r6)
{
    u8 r5;
    struct WindowTemplate template;

    switch (r6->unk80)
    {
    case 0:
        r5 = r4->playerCount - 2;
        BerryCrush_HideTimerSprites(r6);
        memcpy(&template, &gUnknown_082F3324[r4->gameState - 11], sizeof(struct WindowTemplate));
        if (r4->gameState == 13)
            template.height = gUnknown_082F3344[1][r5];
        else
            template.height = gUnknown_082F3344[0][r5];
        r6->unk82 = AddWindow(&template);
        break;
    case 1:
        PutWindowTilemap(r6->unk82);
        FillWindowPixelBuffer(r6->unk82, PIXEL_FILL(0));
        break;
    case 2:
        LoadUserWindowBorderGfx_(r6->unk82, 541, 208);
        DrawStdFrameWithCustomTileAndPalette(r6->unk82, 0, 541, 13);
        break;
    case 3:
        r5 = r4->playerCount - 2;
        switch (r4->gameState)
        {
        case 11:
            PrintTextCentered(r6->unk82, 20, 3, gText_PressesRankings);
            PrintBerryCrushResultWindow(r4, 0, 0xA0, 8 * gUnknown_082F3344[0][r5]);
            r6->unk80 = 5;
            return FALSE;
        case 12:
            PrintTextCentered(r6->unk82, 20, 4, sBCRankingHeaders[r4->unk68.unk20[0][7] + 3]);
            PrintBerryCrushResultWindow(r4, 1, 0xA0, 8 * gUnknown_082F3344[0][r5]);
            r6->unk80 = 5;
            return FALSE;
        case 13:
            PrintTextCentered(r6->unk82, 22, 3, gText_CrushingResults);
            PrintBerryCrushResultWindow(r4, 2, 0xB0, 8 * gUnknown_082F3344[1][r5]);
            break;
        }
        break;
    case 4:
        sub_8021D34(r4);
        break;
    case 5:
        CopyWindowToVram(r6->unk82, 3);
        r6->unk80 = 0;
        return TRUE;
    }
    ++r6->unk80;
    return FALSE;
}

static void sub_802222C(struct BerryCrushGame *r4)
{
    ClearStdWindowAndFrameToTransparent(r4->unk138.unk82, 1);
    RemoveWindow(r4->unk138.unk82);
    sub_8022600(r4);
}

static void Task_ShowBerryCrushRankings(u8 taskId)
{
    u8 i = 0, j, xPos, yPos;
    u32 score = 0;
    s16 *data = gTasks[taskId].data;
    
    switch (data[0])
    {
    case 0:
        data[1] = AddWindow(&sWindowTemplate_BerryCrushRankings);
        PutWindowTilemap(data[1]);
        FillWindowPixelBuffer(data[1], PIXEL_FILL(0));
        LoadUserWindowBorderGfx_(data[1], 541, 208);
        DrawStdFrameWithCustomTileAndPalette(data[1], 0, 541, 13);
        break;
    case 1:
        xPos = 96 - GetStringWidth(1, gText_BerryCrush2, -1) / 2u;
        AddTextPrinterParameterized3(
            data[1],
            1,
            xPos,
            1,
            sBerryCrushTextColorTable[3],
            0,
            gText_BerryCrush2
        );
        xPos = 96 - GetStringWidth(1, gText_PressingSpeedRankings, -1) / 2u;
        AddTextPrinterParameterized3(
            data[1],
            1,
            xPos,
            17,
            sBerryCrushTextColorTable[3],
            0,
            gText_PressingSpeedRankings
        );
        yPos = 41;
        for (i = 0; i < 4; ++i)
        {
            ConvertIntToDecimalStringN(gStringVar1, i + 2, STR_CONV_MODE_LEFT_ALIGN, 1);
            StringExpandPlaceholders(gStringVar4, gText_Var1Players);
            AddTextPrinterParameterized3(
                data[1],
                1,
                0,
                yPos,
                sBerryCrushTextColorTable[0],
                0,
                gStringVar4
            );
            xPos = 192 - (u8)GetStringWidth(1, gText_TimesPerSec, -1);
            AddTextPrinterParameterized3(
                data[1],
                1,
                xPos,
                yPos,
                sBerryCrushTextColorTable[0],
                0,
                gText_TimesPerSec
            );
            for (j = 0; j < 8; ++j)
            {
                if (((data[i + 2] & 0xFF) >> (7 - j)) & 1)
                    score += sPressingSpeedConversionTable[j];
            }
            ConvertIntToDecimalStringN(gStringVar1, (u16)data[i + 2] >> 8, STR_CONV_MODE_RIGHT_ALIGN, 3);
            ConvertIntToDecimalStringN(gStringVar2, score / 1000000, STR_CONV_MODE_LEADING_ZEROS, 2);
            StringExpandPlaceholders(gStringVar4, gText_XDotY3);
            xPos -= GetStringWidth(1, gStringVar4, -1);
            AddTextPrinterParameterized3(
                data[1],
                1,
                xPos,
                yPos,
                sBerryCrushTextColorTable[0],
                0,
                gStringVar4
            );
            yPos += 16;
            score = 0;
        }
        CopyWindowToVram(data[1], 3);
        break;
    case 2:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        else
            return;
    case 3:
        ClearStdWindowAndFrameToTransparent(data[1], 1);
        ClearWindowTilemap(data[1]);
        RemoveWindow(data[1]);
        DestroyTask(taskId);
        EnableBothScriptContexts();
        ScriptContext2_Disable();
        data[0] = 0;
        return;
    }
    ++data[0];
}

void ShowBerryCrushRankings(void)
{
    u8 taskId;

    ScriptContext2_Enable();
    taskId = CreateTask(Task_ShowBerryCrushRankings, 0);
    gTasks[taskId].data[2] = gSaveBlock2Ptr->berryCrush.berryCrushResults[0];
    gTasks[taskId].data[3] = gSaveBlock2Ptr->berryCrush.berryCrushResults[1];
    gTasks[taskId].data[4] = gSaveBlock2Ptr->berryCrush.berryCrushResults[2];
    gTasks[taskId].data[5] = gSaveBlock2Ptr->berryCrush.berryCrushResults[3];
}

static void BerryCrush_PrintTimeOnSprites(struct BerryCrushGame_138 *r4, u16 r1)
{
    FramesToMinSec(r4, r1);
    DigitObjUtil_PrintNumOn(0, r4->minutes);
    DigitObjUtil_PrintNumOn(1, r4->secondsInt);
    DigitObjUtil_PrintNumOn(2, r4->secondsFrac);
}

static void BerryCrush_HideTimerSprites(struct BerryCrushGame_138 *r0)
{
    r0->timerSprites[0]->invisible = TRUE;
    r0->timerSprites[1]->invisible = TRUE;
    DigitObjUtil_HideOrShow(2, 1);
    DigitObjUtil_HideOrShow(1, 1);
    DigitObjUtil_HideOrShow(0, 1);
}

static void sub_8022588(struct BerryCrushGame *r5)
{
    u8 r6;

    for (r6 = 0; r6 < r5->playerCount; ++r6)
    {
        r5->unk138.seatCoords[r6] = &gUnknown_082F4190[gUnknown_082F417C[r5->playerCount - 2][r6]];
        r5->unk138.unk83[r6] = AddWindow(&gUnknown_082F32F4[r5->unk138.seatCoords[r6]->unk0]);
        PutWindowTilemap(r5->unk138.unk83[r6]);
        FillWindowPixelBuffer(r5->unk138.unk83[r6], 0);
    }
}

static void sub_8022600(struct BerryCrushGame *r6)
{
    u8 r7;

    for (r7 = 0; r7 < r6->playerCount; ++r7)
    {
        PutWindowTilemap(r6->unk138.unk83[r7]);
        if (r7 == r6->localId)
        {
            AddTextPrinterParameterized4(
                r6->unk138.unk83[r7],
                2,
                36 - GetStringWidth(2, r6->unk98[r7].unk0, 0) / 2u,
                1,
                0,
                0,
                sBerryCrushTextColorTable[1],
                0,
                r6->unk98[r7].unk0
            );
        }
        else
        {
            AddTextPrinterParameterized4(
                r6->unk138.unk83[r7],
                2,
                36 - GetStringWidth(2, r6->unk98[r7].unk0, 0) / 2u,
                1,
                0,
                0,
                sBerryCrushTextColorTable[2],
                0,
                r6->unk98[r7].unk0
            );
        }
        CopyWindowToVram(r6->unk138.unk83[r7], 3);
    }
    CopyBgTilemapBufferToVram(0);
}

static void sub_80226D0(struct BerryCrushGame *r6)
{
    u8 r5 = 0;
    u8 * r4;

    LZ77UnCompWram(gUnknown_08DE3FD4, gDecompressionBuffer);

    for (r4 = gDecompressionBuffer; r5 < r6->playerCount; ++r5)
    {
        CopyToBgTilemapBufferRect(
            3,
            &r4[r6->unk138.seatCoords[r5]->unk0 * 40],
            r6->unk138.seatCoords[r5]->unk1,
            r6->unk138.seatCoords[r5]->unk2,
            10,
            2
        );
    }
    CopyBgTilemapBufferToVram(3);
}

static void sub_8022730(struct BerryCrushGame *r6)
{
    u8 r5 = 0;
    u8 r2;

    r6->depth = -104;
    r6->vibration = 0;
    gSpriteCoordOffsetX = 0;
    gSpriteCoordOffsetY = -104;
    for (; r5 < 4; ++r5)
        LoadCompressedSpriteSheet(&gUnknown_082F41F4[r5]);
    LoadSpritePalettes(sSpritePals);
    r2 = CreateSprite(&sSpriteTemplate_BerryCrushCore, 120, 88, 5);
    r6->unk138.coreSprite = &gSprites[r2];
    r6->unk138.coreSprite->oam.priority = 3;
    r6->unk138.coreSprite->coordOffsetEnabled = TRUE;
    r6->unk138.coreSprite->animPaused = TRUE;
    for (r5 = 0; r5 < r6->playerCount; ++r5)
    {
        r2 = CreateSprite(
            &sSpriteTemplate_BerryCrushImpact,
            r6->unk138.seatCoords[r5]->unk4 + 120,
            r6->unk138.seatCoords[r5]->unk6 + 32,
            0
        );
        r6->unk138.impactSprites[r5] = &gSprites[r2];
        r6->unk138.impactSprites[r5]->oam.priority = 1;
        r6->unk138.impactSprites[r5]->invisible = TRUE;
        r6->unk138.impactSprites[r5]->coordOffsetEnabled = TRUE;
        r6->unk138.impactSprites[r5]->animPaused = TRUE;
    }
    for (r5 = 0; r5 < ARRAY_COUNT(r6->unk138.sparkleSprites); ++r5)
    {
        r2 = CreateSprite(
            &sSpriteTemplate_BerryCrushPowderSparkles,
            gUnknown_082F41D2[r5][0] + 120,
            gUnknown_082F41D2[r5][1] + 136,
            6
        );
        r6->unk138.sparkleSprites[r5] = &gSprites[r2];
        r6->unk138.sparkleSprites[r5]->oam.priority = 3;
        r6->unk138.sparkleSprites[r5]->invisible = TRUE;
        r6->unk138.sparkleSprites[r5]->animPaused = TRUE;
        r6->unk138.sparkleSprites[r5]->data[0] = r5;
    }
    for (r5 = 0; r5 < ARRAY_COUNT(r6->unk138.timerSprites); ++r5)
    {
        r2 = CreateSprite(
            &sSpriteTemplate_BerryCrushTimer,
            24 * r5 + 176,
            8,
            0
        );
        r6->unk138.timerSprites[r5] = &gSprites[r2];
        r6->unk138.timerSprites[r5]->oam.priority = 0;
        r6->unk138.timerSprites[r5]->invisible = FALSE;
        r6->unk138.timerSprites[r5]->animPaused = FALSE;
    }
    DigitObjUtil_CreatePrinter(0, 0, &sDigitObjTemplates[0]);
    DigitObjUtil_CreatePrinter(1, 0, &sDigitObjTemplates[1]);
    DigitObjUtil_CreatePrinter(2, 0, &sDigitObjTemplates[2]);
    if (r6->gameState == 1)
        BerryCrush_HideTimerSprites(&r6->unk138);
}

static void sub_8022960(struct BerryCrushGame *r5)
{
    u8 r4 = 0;

    FreeSpriteTilesByTag(4);
    FreeSpriteTilesByTag(3);
    FreeSpriteTilesByTag(2);
    FreeSpriteTilesByTag(1);
    FreeSpritePaletteByTag(4);
    FreeSpritePaletteByTag(2);
    FreeSpritePaletteByTag(1);
    for (; r4 < ARRAY_COUNT(r5->unk138.timerSprites); ++r4)
        DestroySprite(r5->unk138.timerSprites[r4]);
    DigitObjUtil_DeletePrinter(2);
    DigitObjUtil_DeletePrinter(1);
    DigitObjUtil_DeletePrinter(0);
    for (r4 = 0; r4 < ARRAY_COUNT(r5->unk138.sparkleSprites); ++r4)
        DestroySprite(r5->unk138.sparkleSprites[r4]);
    for (r4 = 0; r4 < r5->playerCount; ++r4)
        DestroySprite(r5->unk138.impactSprites[r4]);
    if (r5->unk138.coreSprite->inUse)
        DestroySprite(r5->unk138.coreSprite);
}

static void SpriteCB_BerryCrushImpact(struct Sprite *sprite)
{
    if (sprite->animEnded)
    {
        sprite->invisible = TRUE;
        sprite->animPaused = TRUE;
    }
}

static void sub_8022A4C(struct Sprite *sprite)
{
    u8 r1 = 0;
    SpriteCallback r5 = SpriteCallbackDummy;

    for (; r1 < ARRAY_COUNT(sprite->data); ++r1)
        sprite->data[r1] = 0;
    sprite->pos2.x = 0;
    sprite->pos2.y = 0;
    sprite->invisible = TRUE;
    sprite->animPaused = TRUE;
    sprite->callback = r5;
}

static void sub_8022A94(struct Sprite *sprite)
{
    s16 *r4 = sprite->data;

    r4[1] += r4[2];
    sprite->pos2.y += r4[1] >> 8;
    if (r4[7] & 0x8000)
    {
        sprite->data[0] += r4[3];
        r4[4] += r4[5];
        sprite->pos2.x = Sin(r4[4] >> 7, r4[6]);
        if (r4[7] & 0x8000 && r4[4] >> 7 > 126)
        {
            sprite->pos2.x = 0;
            r4[7] &= 0x7FFF;
        }
    }
    sprite->pos1.x = r4[0] >> 7;
    if (sprite->pos1.y + sprite->pos2.y > (r4[7] & 0x7FFF))
        sprite->callback = sub_8022A4C;
}

static void sub_8022B28(struct Sprite *sprite)
{
    s16 *r7 = sprite->data;
    s16 r4, r5;
    s32 r2;
    u32 r8 = 0;

    r2 = 640;
    r7[1] = r2;
    r7[2] = 32;
    r7[7] = 168;
    r4 = sprite->pos2.x * 128;
    r5 = MathUtil_Div16Shift(7, (168 - sprite->pos1.y) << 7, (r2 + 32) >> 1);
    sprite->data[0] = sprite->pos1.x << 7;
    r7[3] = MathUtil_Div16Shift(7, r4, r5);
    r2 = MathUtil_Mul16Shift(7, r5, 85);
    r7[4] = r8;
    r7[5] = MathUtil_Div16Shift(7, Q_8_8(63.5), r2);
    r7[6] = sprite->pos2.x / 4;
    r7[7] |= 0x8000;
    sprite->pos2.y = r8;
    sprite->pos2.x = r8;
    sprite->callback = sub_8022A94;
    sprite->animPaused = FALSE;
    sprite->invisible = FALSE;
}

static void BerryCrush_RunOrScheduleCommand(u16 r5, u8 r4, u8 *r7)
{
    struct BerryCrushGame *r6 = GetBerryCrushGame();

    if (r5 >= ARRAY_COUNT(sBerryCrushCommands))
        r5 = 0;
    switch (r4)
    {
    case 0:
        if (r5 != 0)
            sBerryCrushCommands[r5](r6, r7);
        if (r6->nextCmd >= ARRAY_COUNT(sBerryCrushCommands))
            r6->nextCmd = r4;
        r6->cmdCallback = sBerryCrushCommands[r6->nextCmd];
        break;
    case 1:
        r6->cmdCallback = sBerryCrushCommands[r5];
        break;
    }
}

static u32 BerryCrushCommand_BeginNormalPaletteFade(struct BerryCrushGame *game, u8 *params)
{
    // params points to packed values:
    // bytes 0-3: selectedPals (bitfield)
    // byte 4: delay
    // byte 5: startY
    // byte 6: stopY
    // bytes 7-8: fade color
    // byte 9: if TRUE, communicate on fade complete

    u16 color;
    u32 selectedPals[2];

    selectedPals[0] = (u32)params[0];
    selectedPals[1] = (u32)params[1];
    selectedPals[1] <<= 8;

    selectedPals[0] |= selectedPals[1];
    selectedPals[1] = (u32)params[2];
    selectedPals[1] <<= 16;

    selectedPals[0] |= selectedPals[1];
    selectedPals[1] = (u32)params[3];
    selectedPals[1] <<= 24;

    selectedPals[0] |= selectedPals[1];
    params[0] = params[9];

    color = params[8];
    color <<= 8;
    color |= params[7];

    gPaletteFade.bufferTransferDisabled = FALSE;
    BeginNormalPaletteFade(selectedPals[0], params[4], params[5], params[6], color);
    UpdatePaletteFade();
    game->nextCmd = 2;
    return 0;
}

static u32 BerryCrushCommand_WaitPaletteFade(struct BerryCrushGame *r4, u8 *r5)
{
    switch (r4->cmdState)
    {
    case 0:
        if (UpdatePaletteFade())
            return 0;
        if(r5[0] != 0)
            ++r4->cmdState;
        else
            r4->cmdState = 3;
        return 0;
    case 1:
        Rfu_SetLinkStandbyCallback();
        ++r4->cmdState;
        return 0;
    case 2:
        if (IsLinkTaskFinished())
        {
            ++r4->cmdState;
            return 0;
        }
        return 0;
    case 3:
        BerryCrush_RunOrScheduleCommand(r4->afterPalFadeCmd, 1, NULL);
        r4->cmdState = 0;
        return 0;
    default:
        ++r4->cmdState;
        return 0;
    }
}

static u32 BerryCrushCommand_PrintMessage(struct BerryCrushGame *r7, u8 *r5)
{
    u16 r4 = r5[3];

    r4 <<= 8;
    r4 |= r5[2];
    switch (r7->cmdState)
    {
    case 0:
        DrawDialogueFrame(0, 0);
        if (r5[1] & 2)
        {
            StringExpandPlaceholders(gStringVar4, sBerryCrushMessages[r5[0]]);
            AddTextPrinterParameterized2(0, 1, gStringVar4, r7->textSpeed, 0, 2, 1, 3);
        }
        else
        {
            AddTextPrinterParameterized2(0, 1, sBerryCrushMessages[r5[0]], r7->textSpeed, 0, 2, 1, 3);
        }
        CopyWindowToVram(0, 3);
        break;
    case 1:
        if (!IsTextPrinterActive(0))
        {
            if (r4 == 0)
                ++r7->cmdState;
            break;
        }
        return 0;
    case 2:
        if (!(r4 & gMain.newKeys))
            return 0;
        break;
    case 3:
        if (r5[1] & 1)
            ClearDialogWindowAndFrame(0, 1);
        BerryCrush_RunOrScheduleCommand(r7->nextCmd, 1, NULL);
        r7->cmdState = r5[4];
        return 0;
    }
    ++r7->cmdState;
    return 0;
}

static u32 BerryCrushCommand_InitGfx(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    if (InitBerryCrushDisplay() != 0)
        BerryCrush_RunOrScheduleCommand(r4->nextCmd, 0, r4->unk36);
    return 0;
}

static u32 BerryCrushCommand_TeardownGfx(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    if (BerryCrush_TeardownBgs() != 0)
        BerryCrush_RunOrScheduleCommand(r4->nextCmd, 0, r4->unk36);
    return 0;
}

static u32 BerryCrushCommand_SignalReadyToBegin(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    switch (r4->cmdState)
    {
    case 0:
        Rfu_SetLinkStandbyCallback();
        break;
    case 1:
        if (IsLinkTaskFinished())
        {
            PlayNewMapMusic(MUS_RG_GAME_CORNER);
            BerryCrush_RunOrScheduleCommand(7, 1, NULL);
            r4->gameState = 3;
            r4->cmdState = 0;
        }
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

static u32 BerryCrushCommand_AskPickBerry(struct BerryCrushGame *r4, u8 *r5)
{
    switch (r4->cmdState)
    {
    default:
        ++r4->cmdState;
        break;
    case 0:
        sub_8024578(r4);
        BerryCrush_SetShowMessageParams(r5, 0, 1, 0, 1);
        r4->nextCmd = 7;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        break;
    case 1:
        r4->nextCmd = 8;
        BerryCrush_RunOrScheduleCommand(5, 1, NULL);
        r4->cmdState = 2;
        break;
    }
    return 0;
}

static u32 BerryCrushCommand_GoToBerryPouch(struct BerryCrushGame *r0, __attribute__((unused)) u8 *r1)
{
    r0->cmdCallback = NULL;
    SetMainCallback2(BerryCrush_SetupMainTask);
    return 0;
}

static u32 BerryCrushCommand_WaitForOthersToPickBerries(struct BerryCrushGame *r5, u8 *r2)
{
    u8 r3;

    switch (r5->cmdState)
    {
    case 0:
        BerryCrush_SetShowMessageParams(r2, 1, 0, 0, 1);
        r5->nextCmd = 9;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        return 0;
    case 1:
        Rfu_SetLinkStandbyCallback();
        break;
    case 2:
        if (!IsLinkTaskFinished())
            return 0;
        memset(r5->unk42, 0, sizeof(r5->unk42));
        r5->unk42[0] = r5->unk98[r5->localId].unkC;
        SendBlock(0, r5->unk42, 2);
        break;
    case 3:
        if (!IsLinkTaskFinished())
            return 0;
        r5->unk10 = 0;
        break;
    case 4:
        if (GetBlockReceivedStatus() != sReceivedPlayerBitmasks[r5->playerCount - 2])
            return 0;
        for (r3 = 0; r3 < r5->playerCount; ++r3)
        {
            r5->unk98[r3].unkC = gBlockRecvBuffer[r3][0];
            if (r5->unk98[r3].unkC > 0xB0)
                r5->unk98[r3].unkC = 0;
            r5->unk18 += gUnknown_0858AB24[r5->unk98[r3].unkC].unk0;
            r5->powder += gUnknown_0858AB24[r5->unk98[r3].unkC].unk1;
        }
        r5->unk10 = 0;
        ResetBlockReceivedFlags();
        r5->unk20 = MathUtil_Div32(Q_24_8(r5->unk18), Q_24_8(32));
        break;
    case 5:
        ClearDialogWindowAndFrame(0, 1);
        BerryCrush_RunOrScheduleCommand(10, 1, NULL);
        r5->gameState = 4;
        r5->cmdState = 0;
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_DropBerriesIntoCrusher(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1)
{
    switch (r4->cmdState)
    {
    case 0:
        BerryCrush_CreateBerrySprites(r4, &r4->unk138);
        Rfu_SetLinkStandbyCallback();
        break;
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        r4->unk138.animBerryIdx = 0;
        r4->unk138.unk1 = 0;
        r4->unk138.unk2 = 0;
        r4->unk138.unk3 = 0;
        break;
    case 2:
        r4->unk138.berrySprites[r4->unk138.animBerryIdx]->callback = SpriteCB_DropBerryIntoCrusher;
        r4->unk138.berrySprites[r4->unk138.animBerryIdx]->affineAnimPaused = FALSE;
        PlaySE(SE_BALL_THROW);
        break;
    case 3:
        if (r4->unk138.berrySprites[r4->unk138.animBerryIdx]->callback == SpriteCB_DropBerryIntoCrusher)
            return 0;
        r4->unk138.berrySprites[r4->unk138.animBerryIdx] = NULL;
        ++r4->unk138.animBerryIdx;
        Rfu_SetLinkStandbyCallback();
        break;
    case 4:
        if (!IsLinkTaskFinished())
            return 0;
        if (r4->unk138.animBerryIdx < r4->playerCount)
        {
            r4->cmdState = 2;
            return 0;
        }
        r4->unk138.animBerryIdx = 0;
        break;
    case 5:
        BerryCrushFreeBerrySpriteGfx(r4, &r4->unk138);
        Rfu_SetLinkStandbyCallback();
        break;
    case 6:
        if (!IsLinkTaskFinished())
            return 0;
        PlaySE(SE_FALL);
        BerryCrush_RunOrScheduleCommand(11, 1, NULL);
        r4->gameState = 5;
        r4->cmdState = 0;
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

static u32 BerryCrushCommand_DropLid(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1)
{
    switch (r4->cmdState)
    {
    case 0:
        r4->depth += 4;
        if (r4->depth < 0)
            return 0;
        r4->depth = 0;
        r4->unk138.unk1 = 4;
        r4->unk138.animBerryIdx = 0;
        r4->unk138.unk2 = gUnknown_082F326C[r4->unk138.unk1][0];
        PlaySE(SE_M_STRENGTH);
        break;
    case 1:
        r4->vibration = gUnknown_082F326C[r4->unk138.unk1][r4->unk138.animBerryIdx];
        SetGpuReg(REG_OFFSET_BG0VOFS, -r4->vibration);
        SetGpuReg(REG_OFFSET_BG2VOFS, -r4->vibration);
        SetGpuReg(REG_OFFSET_BG3VOFS, -r4->vibration);
        ++r4->unk138.animBerryIdx;
        if (r4->unk138.animBerryIdx < r4->unk138.unk2)
            return 0;
        if (r4->unk138.unk1 == 0)
            break;
        --r4->unk138.unk1;
        r4->unk138.unk2 = gUnknown_082F326C[r4->unk138.unk1][0];
        r4->unk138.animBerryIdx = 0;
        return 0;
    case 2:
        r4->vibration = 0;
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
        Rfu_SetLinkStandbyCallback();
        break;
    case 3:
        if (!IsLinkTaskFinished())
            return 0;
        BerryCrush_RunOrScheduleCommand(12, 1, NULL);
        r4->gameState = 6;
        r4->cmdState = 0;
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

static u32 BerryCrushCommand_Countdown(struct BerryCrushGame *r4,  __attribute__((unused)) u8 *r1)
{
    switch (r4-> cmdState)
    {
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        StartMinigameCountdown(0x1000, 0x1000, 120, 80, 0);
        break;
    case 2:
        if (IsMinigameCountdownRunning())
            return 0;
        // fallthrough
    case 0:
        Rfu_SetLinkStandbyCallback();
        break;
    case 3:
        if (!IsLinkTaskFinished())
            return 0;
        r4->unk138.animBerryIdx = 0;
        r4->unk138.unk1 = 0;
        r4->unk138.unk2 = 0;
        r4->unk138.unk3 = 0;
        r4->unk10 = 0;
        if (r4->localId == 0)
            BerryCrush_RunOrScheduleCommand(13, 1, NULL);
        else
            BerryCrush_RunOrScheduleCommand(14, 1, NULL);
        r4->gameState = 7;
        r4->cmdState = 0;
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

void BerryCrush_ProcessGamePartnerInput(struct BerryCrushGame *r4)
{
    u8 r8 = 0;
    u8 r7 = 0;
    u16 r3;
    s32 r2_ = 0;
    struct BerryCrushGame_4E *r2;

    for (r7 = 0; r7 < r4->playerCount; r7++)
    {
        r2 = (struct BerryCrushGame_4E *)gRecvCmds[r7];
        if ((r2->unk0 & 0xFF00) != RFUCMD_SEND_PACKET)
            continue;
        if (r2->unk2 != 2)
            continue;
        
        if (r2->unk4_2)
        {
            r4->localState.unk02_3 |= gUnknown_082F325C[r7];
            r4->unk98[r7].unk1C = 1;
            ++r4->unk98[r7].unk16;
            ++r8;
            r3 = r4->timer - r4->unk98[r7].unkE;
            if (r3 >= r4->unk98[r7].unk12 - 1 && r3 <= r4->unk98[r7].unk12 + 1)
            {
                ++r4->unk98[r7].unk10;
                r4->unk98[r7].unk12 = r3;
                if (r4->unk98[r7].unk10 > r4->unk98[r7].unk14)
                    r4->unk98[r7].unk14 = r4->unk98[r7].unk10;
            }
            else
            {
                r4->unk98[r7].unk10 = 0;
                r4->unk98[r7].unk12 = r3;
            }
            r4->unk98[r7].unkE = r4->timer;
            ++r4->unk98[r7].unk1B;
            if (r4->unk98[r7].unk1B > 2)
                r4->unk98[r7].unk1B = 0;
        }
        else
        {
            r4->unk98[r7].unk1C = 0;
        }
    }
    if (r8 > 1)
    {
        for (r7 = 0; r7 < r4->playerCount; ++r7)
        {
            if (!r4->unk98[r7].unk1C)
                continue;
            r4->unk98[r7].unk1C |= 2;
            ++r4->unk98[r7].unk18;
        }
    }
    if (r8 == 0)
        return;

    r4->unk2E += r8;
    r8 += gUnknown_082F3264[r8 - 1];
    r4->unk34 += r8;
    r4->unk1A += r8;
    if (r4->unk18 - r4->unk1A > 0)
    {
        r2_ = (s32)r4->unk1A;
        r2_ <<= 8;
        r2_ = MathUtil_Div32(r2_, r4->unk20);
        r2_ >>= 8;
        r4->unk24 = (u8)r2_;
        return;
    }

    r4->unk24 = 32;
    r4->localState.unk02_0 = 1;    
}

void BerryCrush_BuildLocalState(struct BerryCrushGame *r3)
{
    u8 r6 = 0;
    u16 r1 = 0;
    u16 r2 = 0;
    u8 r4 = 0;
    
    for (r4 = 0; r4 < r3->playerCount; ++r4)
    {
        if (r3->unk98[r4].unk1C != 0)
        {
            ++r6;
            r1 = r3->unk98[r4].unk1B + 1;
            if (r3->unk98[r4].unk1C & 2)
                r1 |= 4;
            r1 <<= 3 * r4;
            r3->localState.unk08 |= r1;
        }
    }
    r2 = (u16)r3->unk24;
    r3->localState.unk04 = r2;
    if (r6 == 0)
    {
        if (r3->unk138.unk3 != 0)
            ++r3->unk138.animBerryIdx;
    }
    else if (r3->unk138.unk3 != 0)
    {
        if (r6 != r3->unk138.unk1)
        {
            r3->unk138.unk1 = r6 - 1;
            r3->unk138.unk2 = gUnknown_082F3290[r6 - 1][0];
        }
        else
        {
            ++r3->unk138.animBerryIdx;
        }
    }
    else
    {
        r3->unk138.animBerryIdx = 0;
        r3->unk138.unk1 = r6 - 1;
        r3->unk138.unk2 = gUnknown_082F3290[r6 - 1][0];
        r3->unk138.unk3 = 1;
    }

    if (r3->unk138.unk3 != 0)
    {
        if (r3->unk138.animBerryIdx >= r3->unk138.unk2)
        {
            r3->unk138.animBerryIdx = 0;
            r3->unk138.unk1 = 0;
            r3->unk138.unk2 = 0;
            r3->unk138.unk3 = 0;
            r1 = 0;
        }
        else
        {
            r1 = gUnknown_082F3290[r3->unk138.unk1][r3->unk138.animBerryIdx + 1];
        }
        r3->localState.unk03 = (u8)r1;
    }
    else
    {
        r3->localState.unk03 = 0;
    }
    r3->localState.unk06 = r3->unk26;
}

void BerryCrush_HandlePlayerInput(struct BerryCrushGame *r5)
{
    if (JOY_NEW(A_BUTTON))
        r5->localState.pushedAButton = 1;
    if (JOY_HELD(A_BUTTON))
    {
        if (r5->unk98[r5->localId].unk1A < r5->timer)
            ++r5->unk98[r5->localId].unk1A;
    }
    if (r5->localId != 0 && r5->localState.pushedAButton == 0)
        return;
    r5->localState.unk00 = 2;
    if (r5->timer % 30 == 0)
    {
        if (r5->unk2E > gUnknown_082F4444[r5->playerCount - 2])
        {
            ++r5->unk30;
            r5->unk25_4 = 1;
        }
        else
        {
            r5->unk25_4 = 0;
        }
        r5->unk2E = 0;
        ++r5->unk32;
    }
    if (r5->timer % 15 == 0)
    {
        if (r5->unk34 < gUnknown_082F4434[r5->playerCount - 2][0])
            r5->unk25_5 = 0;
        else if (r5->unk34 < gUnknown_082F4434[r5->playerCount - 2][1])
            r5->unk25_5 = 1;
        else if (r5->unk34 < gUnknown_082F4434[r5->playerCount - 2][2])
            r5->unk34 = 2; // typo since r5->unk34 will be reset? 
        else if (r5->unk34 < gUnknown_082F4434[r5->playerCount - 2][3])
            r5->unk34 = 3; // typo since r5->unk34 will be reset? 
        else
            r5->unk25_5 = 4;
        r5->unk34 = 0;
    }
    else
    {
        ++r5->unk10;
        if (r5->unk10 > 60)
        {
            if (r5->unk10 > 70)
            {
                ClearRecvCommands();
                r5->unk10 = 0;
            }
            else if (r5->localState.unk02_3 == 0)
            {
                ClearRecvCommands();
                r5->unk10 = 0;
            }
        }
        
    }
    if (r5->timer >= 36000)
        r5->localState.unk02_0 = 1;
    r5->localState.unk02_1 = r5->unk25_4;
    r5->localState.unk0A = r5->unk25_5;
    memcpy(r5->unk42, &r5->localState, sizeof(r5->unk42));
    Rfu_SendPacket(r5->unk42);
}

void BerryCrush_UpdateGameState(struct BerryCrushGame *r5)
{
    u8 r4 = 0;
    struct BerryCrushGame_4E *r4_ = NULL;

    for (r4 = 0; r4 < r5->playerCount; r4++)
        r5->unk98[r4].unk1C = 0;
    if ((gRecvCmds[0][0] & 0xFF00) != RFUCMD_SEND_PACKET)
    {
        r5->unk25_2 = 0;
        return;
    }
    if (gRecvCmds[0][1] != 2)
    {
        r5->unk25_2 = 0;
        return;
    }

    memcpy(r5->recvCmd, gRecvCmds[0], 14);
    r4_ = (struct BerryCrushGame_4E *)&r5->recvCmd;
    r5->depth = r4_->unk6;
    r5->vibration = (s16)r4_->unk5;
    r5->timer = r4_->unk8;
    sub_80216E0(r5, &(r5->unk138));
    if (r4_->unk4_0)
    {
        r5->unk25_3 = 1;
    }
}

static u32 BerryCrushCommand_PlayGame_Master(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    memset(&r4->localState, 0, sizeof(r4->localState));
    memset(&r4->recvCmd, 0, sizeof(r4->recvCmd));
    BerryCrush_UpdateGameState(r4);
    SetGpuReg(REG_OFFSET_BG0VOFS, -r4->vibration);
    SetGpuReg(REG_OFFSET_BG2VOFS, -r4->vibration);
    SetGpuReg(REG_OFFSET_BG3VOFS, -r4->vibration);
    if (r4->unk25_3)
    {
        if (r4->timer >= 36000)
        {
            r4->timer = 36000;
            BerryCrush_RunOrScheduleCommand(16, 1, NULL);
        }
        else
        {
            BerryCrush_RunOrScheduleCommand(15, 1, NULL);
        }
        r4->unk10 = 0;
        r4->cmdState = 0;
        return 0;
    }
    else
    {
        ++r4->unk26;
        BerryCrush_ProcessGamePartnerInput(r4);
        BerryCrush_BuildLocalState(r4);
        BerryCrush_HandlePlayerInput(r4);
        return 0;
    }
}

static u32 BerryCrushCommand_PlayGame_Slave(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    memset(&r4->localState, 0, sizeof(r4->localState));
    memset(&r4->recvCmd, 0, sizeof(r4->recvCmd));
    BerryCrush_UpdateGameState(r4);
    SetGpuReg(REG_OFFSET_BG0VOFS, -r4->vibration);
    SetGpuReg(REG_OFFSET_BG2VOFS, -r4->vibration);
    SetGpuReg(REG_OFFSET_BG3VOFS, -r4->vibration);
    if (r4->unk25_3)
    {
        if (r4->timer >= 36000)
        {
            r4->timer = 36000;
            BerryCrush_RunOrScheduleCommand(16, 1, NULL);
        }
        else
        {
            BerryCrush_RunOrScheduleCommand(15, 1, NULL);
        }
        r4->unk10 = 0;
        r4->cmdState = 0;
        return 0;
    }
    else
    {
        BerryCrush_HandlePlayerInput(r4);
        return 0;
    }
}

static u32 BerryCrushCommand_FinishGame(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    switch (r4->cmdState)
    {
    case 0:
        r4->gameState = 8;
        PlaySE(SE_M_STRENGTH);
        BlendPalettes(PALETTES_ALL, 8, RGB(31, 31, 0));
        r4->unk138.animBerryIdx = 2;
        break;
    case 1:
        if (--r4->unk138.animBerryIdx != 255)
            return 0;
        BlendPalettes(PALETTES_ALL, 0, RGB(31, 31, 0));
        r4->unk138.unk1 = 4;
        r4->unk138.animBerryIdx = 0;
        r4->unk138.unk2 = gUnknown_082F326C[r4->unk138.unk1][0];
        break;
    case 2:
        r4->vibration = gUnknown_082F326C[r4->unk138.unk1][r4->unk138.animBerryIdx];
        SetGpuReg(REG_OFFSET_BG0VOFS, -r4->vibration);
        SetGpuReg(REG_OFFSET_BG2VOFS, -r4->vibration);
        SetGpuReg(REG_OFFSET_BG3VOFS, -r4->vibration);
        if (++r4->unk138.animBerryIdx < r4->unk138.unk2)
            return 0;
        if (r4->unk138.unk1 != 0)
        {
            --r4->unk138.unk1;
            r4->unk138.unk2 = gUnknown_082F326C[r4->unk138.unk1][0];
            r4->unk138.animBerryIdx = 0;
            return 0;
        }
        break;
    case 3:
        r4->vibration = 0;
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
        break;
    case 4:
        if (!sub_80218D4(r4, &r4->unk138))
            return 0;
        Rfu_SetLinkStandbyCallback();
        r4->unk10 = 0;
        break;
    case 5:
        if (!IsLinkTaskFinished())
            return 0;
        BerryCrush_RunOrScheduleCommand(17, 1, NULL);
        r4->unk10 = 0;
        r4->cmdState = 0;
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

static u32 BerryCrushCommand_HandleTimeUp(struct BerryCrushGame *r5, u8 *r6)
{
    switch (r5->cmdState)
    {
    case 0:
        r5->gameState = 9;
        PlaySE(SE_FAILURE);
        BlendPalettes(PALETTES_ALL, 8, RGB(31, 0, 0));
        r5->unk138.animBerryIdx = 4;
        break;
    case 1:
        if (--r5->unk138.animBerryIdx != 255)
            return 0;
        BlendPalettes(PALETTES_ALL, 0, RGB(31, 0, 0));
        r5->unk138.animBerryIdx = 0;
        break;
    case 2:
        if (!sub_80218D4(r5, &r5->unk138))
            return 0;
        Rfu_SetLinkStandbyCallback();
        r5->unk10 = 0;
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
        break;
    case 3:
        if (!IsLinkTaskFinished())
            return 0;
        ConvertIntToDecimalStringN(gStringVar1, r5->powder, STR_CONV_MODE_LEFT_ALIGN, 6);
        BerryCrush_SetShowMessageParams(r6, 7, 1, 0, 0);
        r5->nextCmd = 19;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        r5->unk10 = 0;
        r5->cmdState = 0;
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_TabulateResults(struct BerryCrushGame *r7, __attribute__((unused)) u8 *r1)
{
    u8 r8, r4_, r3;
    s32 r2;
    s32 r4;
    u16 r6;

    switch (r7->cmdState)
    {
    case 0:
        memset(r7->unk42, 0, 2 * sizeof(u16));
        if (r7->unk98[r7->localId].unk1A > r7->timer)
            r7->unk98[r7->localId].unk1A = r7->timer;
        r7->unk42[0] = r7->unk98[r7->localId].unk1A;
        SendBlock(0, r7->unk42, 2);
        break;
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        r7->unk10 = 0;
        break;
    case 2:
        if (GetBlockReceivedStatus() != sReceivedPlayerBitmasks[r7->playerCount - 2])
            return 0;
        for (r8 = 0; r8 < r7->playerCount; ++r8)
            r7->unk98[r8].unk1A = gBlockRecvBuffer[r8][0];
        r7->unk10 = 0;
        r7->unk42[0] = 0;
        ResetBlockReceivedFlags();
        if (r7->localId == 0)
            r7->cmdState = 3;
        else
            r7->cmdState = 6;
        return 0;
    case 3:
        memset(&r7->unk68, 0, sizeof(struct BerryCrushGame_68));
        r7->unk68.unk04 = r7->timer;
        r7->unk68.unk06 = r7->unk18 / (r7->timer / 60);
        r2 = MathUtil_Mul32(Q_24_8(r7->unk30), Q_24_8(50));
        r2 = MathUtil_Div32(r2, Q_24_8(r7->unk32)) + Q_24_8(50);
        r2 = Q_24_8_TO_INT(r2);
        r7->unk68.unk08 = r2 & 0x7F;
        r2 = Q_24_8(r2);
        r2 = MathUtil_Div32(r2, Q_24_8(100));
        r4 = Q_24_8(r7->powder * r7->playerCount);
        r4 = MathUtil_Mul32(r4, r2);
        r7->unk68.unk00 = r4 >> 8;
        r7->unk68.unk20[0][7] = Random() % 3;
        for (r8 = 0; r8 < r7->playerCount; ++r8)
        {
            r7->unk68.unk20[0][r8] = r8;
            r7->unk68.unk20[1][r8] = r8;
            r7->unk68.stats[0][r8] = r7->unk98[r8].unk16;
            r7->unk68.unk0A += r7->unk68.stats[0][r8];
            switch (r7->unk68.unk20[0][7])
            {
            case 0:
                if (r7->unk98[r8].unk16 != 0)
                {
                    r2 = r7->unk98[r8].unk14;
                    r2 = Q_24_8(r2);
                    r2 = MathUtil_Mul32(r2, Q_24_8(100));
                    r4 = r7->unk98[r8].unk16;
                    r4 = Q_24_8(r4);
                    r4 = MathUtil_Div32(r2, r4);
                }
                else
                {
                    r4 = 0;
                }
                break;
            case 1:
                if (r7->unk98[r8].unk16 != 0)
                {
                    r2 = r7->unk98[r8].unk18;
                    r2 = Q_24_8(r2);
                    r2 = MathUtil_Mul32(r2, Q_24_8(100));
                    r4 = r7->unk98[r8].unk16;
                    r4 = Q_24_8(r4);
                    r4 = MathUtil_Div32(r2, r4);
                }
                else
                {
                    r4 = 0;
                }
                break;
            case 2:
                if (r7->unk98[r8].unk16 == 0)
                {
                    r4 = 0;
                }
                else if (r7->unk98[r8].unk1A >= r7->timer)
                {
                    r4 = 0x6400;
                }
                else
                {
                    r2 = r7->unk98[r8].unk1A;
                    r2 = Q_24_8(r2);
                    r2 = MathUtil_Mul32(r2, Q_24_8(100));
                    r4 = r7->timer;
                    r4 = Q_24_8(r4);
                    r4 = MathUtil_Div32(r2, r4);
                }
                break;
            }
            r4 >>= 4;
            r7->unk68.stats[1][r8] = r4;
        }
        break;
    case 4:
        for (r8 = 0; r8 < r7->playerCount - 1; ++r8)
        {
            for (r4_ = r7->playerCount - 1; r4_ > r8; --r4_)
            {
                if (r7->unk68.stats[0][r4_ - 1] < r7->unk68.stats[0][r4_])
                {
                    r6 = r7->unk68.stats[0][r4_];
                    r7->unk68.stats[0][r4_] = r7->unk68.stats[0][r4_ - 1];
                    r7->unk68.stats[0][r4_ - 1] = r6;
                    r3 = r7->unk68.unk20[0][r4_];
                    r7->unk68.unk20[0][r4_] = r7->unk68.unk20[0][r4_ - 1];
                    r7->unk68.unk20[0][r4_ - 1] = r3;
                }
                if (r7->unk68.stats[1][r4_ - 1] < r7->unk68.stats[1][r4_])
                {
                    r6 = r7->unk68.stats[1][r4_];
                    r7->unk68.stats[1][r4_] = r7->unk68.stats[1][r4_ - 1];
                    r7->unk68.stats[1][r4_ - 1] = r6;
                    r3 = r7->unk68.unk20[1][r4_];
                    r7->unk68.unk20[1][r4_] = r7->unk68.unk20[1][r4_ - 1];
                    r7->unk68.unk20[1][r4_ - 1] = r3;
                }
            }
        }
        SendBlock(0,&r7->unk68, sizeof(struct BerryCrushGame_68));
        break;
    case 5:
        if (!IsLinkTaskFinished())
            return 0;
        r7->unk10 = 0;
        break;
    case 6:
        if (GetBlockReceivedStatus() != 1)
            return 0;
        memset(&r7->unk68, 0, sizeof(struct BerryCrushGame_68));
        memcpy(&r7->unk68, gBlockRecvBuffer, sizeof(struct BerryCrushGame_68));
        ResetBlockReceivedFlags();
        r7->unk10 = 0;
        break;
    case 7:
        BerryCrush_SaveResults();
        BerryCrush_RunOrScheduleCommand(18, 1, NULL);
        r7->gameState = 11;
        r7->cmdState = 0;
        r7->unk24 = 0;
        return 0;
    }
    ++r7->cmdState;
    return 0;
}

static u32 BerryCrushCommand_ShowResults(struct BerryCrushGame *r5, u8 *r6)
{
    switch (r5->cmdState)
    {
    case 0:
        if (!sub_8022070(r5, &r5->unk138))
            return 0;
        break;
    case 1:
        CopyBgTilemapBufferToVram(0);
        r5->unk138.animBerryIdx = 30;
        break;
    case 2:
        if (r5->unk138.animBerryIdx != 0)
        {
            --r5->unk138.animBerryIdx;
            return 0;
        }
        if (!(JOY_NEW(A_BUTTON)))
            return 0;
        PlaySE(SE_SELECT);
        sub_802222C(r5);
        break;
    case 3:
        if (r5->gameState <= 12)
        {
            ++r5->gameState;
            r5->cmdState = 0;
            return 0;
        }
        break;
    case 4:
        ConvertIntToDecimalStringN(gStringVar1, r5->powder, STR_CONV_MODE_LEFT_ALIGN, 6);
        ConvertIntToDecimalStringN(gStringVar2, GetBerryPowder(), STR_CONV_MODE_LEFT_ALIGN, 6);
        BerryCrush_SetShowMessageParams(r6, 2, 3, 0, 0);
        r5->nextCmd = 19;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        r5->cmdState = 0;
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_SaveGame(struct BerryCrushGame *r5, u8 *r4)
{
    switch (r5->cmdState)
    {
    case 0:
        if (r5->timer >= 36000)
            BerryCrush_HideTimerSprites(&r5->unk138);
        BerryCrush_SetShowMessageParams(r4, 8, 0, 0, 1);
        r5->nextCmd = 19;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        r5->cmdState = 0;
        return 0;
    case 1:
        Rfu_SetLinkStandbyCallback();
        break;
    case 2:
        if (!IsLinkTaskFinished())
            return 0;
        DrawDialogueFrame(0, 0);
        AddTextPrinterParameterized2(0, 1, gText_SavingDontTurnOffPower, 0, 0, 2, 1, 3);
        CopyWindowToVram(0, 3);
        CreateTask(Task_LinkSave, 0);
        break;
    case 3:
        if (FuncIsActiveTask(Task_LinkSave))
            return 0;
        break;
    case 4:
        BerryCrush_RunOrScheduleCommand(20, 1, NULL);
        r5->gameState = 15;
        r5->cmdState = 0;
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_AskPlayAgain(struct BerryCrushGame *r5, u8 *r6)
{
    s8 r4 = 0;

    switch (r5->cmdState)
    {
    case 0:
        BerryCrush_SetShowMessageParams(r6, 4, 0, 0, 1);
        r5->nextCmd = 20;
        BerryCrush_RunOrScheduleCommand(3, 1, NULL);
        r5->cmdState = 0; // dunno what it's doing because it's already in case 0
        return 0;
    case 1:
        DisplayYesNoMenuDefaultYes();
        break;
    case 2:
        r4 = Menu_ProcessInputNoWrapClearOnChoose();
        if (r4 != -2)
        {
            memset(r5->unk42, 0, sizeof(r5->unk42));
            if (r4 == 0)
            {
                if (HasAtLeastOneBerry())
                    r5->unk14 = 0;
                else
                    r5->unk14 = 3;
            }
            else
            {
                r5->unk14 = 1;
            }
            ClearDialogWindowAndFrame(0, 1);
            BerryCrush_SetShowMessageParams(r6, 8, 0, 0, 0);
            r5->nextCmd = 21;
            BerryCrush_RunOrScheduleCommand(3, 1, NULL);
            r5->cmdState = 0;
        }
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_CommunicatePlayAgainResponses(struct BerryCrushGame *r4, __attribute__((unused)) u8 *r1)
{
    u8 r5 = 0;

    switch (r4->cmdState)
    {
    case 0:
        Rfu_SetLinkStandbyCallback();
        break;
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        r4->unk42[0] = r4->unk14;
        r4->recvCmd[0] = 0;
        SendBlock(0, r4->unk42, sizeof(u16));
        break;
    case 2:
        if (!IsLinkTaskFinished())
            return 0;
        r4->unk10 = 0;
        break;
    case 3:
        if (GetBlockReceivedStatus() != sReceivedPlayerBitmasks[r4->playerCount - 2])
            return 0;
        for (; r5 < r4->playerCount; ++r5)
            r4->recvCmd[0] += gBlockRecvBuffer[r5][0];
        if (r4->recvCmd[0] != 0)
            BerryCrush_RunOrScheduleCommand(23, 1, NULL);
        else
            BerryCrush_RunOrScheduleCommand(22, 1, NULL);
        ResetBlockReceivedFlags();
        r4->unk42[0] = 0;
        r4->recvCmd[0] = 0;
        r4->unk10 = 0;
        r4->cmdState = 0;
        return 0;
    }
    ++r4->cmdState;
    return 0;
}

static u32 BerryCrushCommand_FadeOutToPlayAgain(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1)
{
    switch (r5->cmdState)
    {
    case 0:
        BeginNormalPaletteFade(PALETTES_ALL, 1, 0, 0x10, RGB_BLACK);
        UpdatePaletteFade();
        break;
    case 1:
        if (UpdatePaletteFade())
            return 0;
        break;
    case 2:
        ClearDialogWindowAndFrame(0, 1);
        sub_8021488(r5);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        UpdatePaletteFade();
        break;
    case 3:
        if (UpdatePaletteFade())
            return 0;
        BerryCrush_RunOrScheduleCommand(7, 1, NULL);
        r5->gameState = 3;
        r5->cmdState = 0;
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_PlayAgainFailureMessage(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1)
{
    switch (r5->cmdState)
    {
    case 0:
        DrawDialogueFrame(0, 0);
        if (r5->unk14 == 3)
            AddTextPrinterParameterized2(0, 1, sBerryCrushMessages[5], r5->textSpeed, 0, 2, 1, 3);
        else
            AddTextPrinterParameterized2(0, 1, sBerryCrushMessages[6], r5->textSpeed, 0, 2, 1, 3);
        CopyWindowToVram(0, 3);
        break;
    case 1:
        if (IsTextPrinterActive(0))
            return 0;
        r5->unk138.animBerryIdx = 120;
        break;
    case 2:
        if (r5->unk138.animBerryIdx != 0)
            --r5->unk138.animBerryIdx;
        else
        {
            BerryCrush_RunOrScheduleCommand(24, 1, NULL);
            r5->cmdState = 0;
        }
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_GracefulExit(struct BerryCrushGame *r5, __attribute__((unused)) u8 *r1)
{
    switch (r5->cmdState)
    {
    case 0:
        Rfu_SetLinkStandbyCallback();
        break;
    case 1:
        if (!IsLinkTaskFinished())
            return 0;
        SetCloseLinkCallback();
        break;
    case 2:
        if (gReceivedRemoteLinkPlayers != 0)
            return 0;
        r5->nextCmd = 25;
        BerryCrush_RunOrScheduleCommand(5, 1, NULL);
        r5->cmdState = 2; // ???
        return 0;
    }
    ++r5->cmdState;
    return 0;
}

static u32 BerryCrushCommand_Quit(__attribute__((unused)) struct BerryCrushGame *r0, __attribute__((unused)) u8 *r1)
{
    QuitBerryCrush(NULL);
    return 0;
}

static void sub_8024578(struct BerryCrushGame *r4)
{
    u8 r5 = 0;

    IncrementGameStat(GAME_STAT_51);
    r4->unkD = 0;
    r4->unk10 = 0;
    r4->gameState = 2;
    r4->unk14 = 0;
    r4->powder = 0;
    r4->unk18 = 0;
    r4->unk1A = 0;
    r4->unk20 = 0;
    r4->unk24 = 0;
    r4->unk25_0 = 0;
    r4->unk25_1 = 0;
    r4->unk25_2 = 0;
    r4->unk25_3 = 0;
    r4->unk25_4 = 0;
    r4->unk25_5 = 0;
    r4->unk26 = 0;
    r4->timer = 0;
    r4->unk2E = 0;
    r4->unk32 = -1;
    r4->unk30 = 0;
    r4->unk34 = 0;
    for (; r5 < 5; ++r5)
    {
        r4->unk98[r5].unkC = -1;
        r4->unk98[r5].unkE = 0;
        r4->unk98[r5].unk10 = 0;
        r4->unk98[r5].unk12 = 1;
        r4->unk98[r5].unk14 = 0;
        r4->unk98[r5].unk16 = 0;
        r4->unk98[r5].unk18 = 0;
        r4->unk98[r5].unk1A = 0;
        r4->unk98[r5].unk1B = 0;
        r4->unk98[r5].unk1C = 0;
    }
}

static void BerryCrush_SetPaletteFadeParams(u8 *params, bool8 communicateAfter, u32 selectedPals, s8 delay, u8 startY, u8 targetY, u16 palette)
{
    params[0] = ((u8 *)&selectedPals)[0];
    params[1] = ((u8 *)&selectedPals)[1];
    params[2] = ((u8 *)&selectedPals)[2];
    params[3] = ((u8 *)&selectedPals)[3];
    params[4] = delay;
    params[5] = startY;
    params[6] = targetY;
    params[7] = ((u8 *)&palette)[0];
    params[8] = ((u8 *)&palette)[1];
    params[9] = communicateAfter;
}

static void BerryCrush_SetShowMessageParams(u8 *params, u8 stringId, u8 flags, u16 waitKeys, u8 followupCmd)
{
    params[0] = stringId;
    params[1] = flags;
    params[2] = ((u8 *)&waitKeys)[0];
    params[3] = ((u8 *)&waitKeys)[1];
    params[4] = followupCmd;
}
