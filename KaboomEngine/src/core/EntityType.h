#pragma once

enum EntityCategory {
    CAT_MASK = 15 << 16,
    CAT_CHARACTER = 1 << 16,
    CAT_BOMB = 2 << 16,
    CAT_POWERUP = 3 << 16,
	CAT_JUMPPAD = 4 << 16
};

enum EntityType {
    NONE = 0,
    DEFAULT_CHARACTER = CAT_CHARACTER | 1,
    KABOOM_V2 = CAT_BOMB | 1,
    TIME_BOMB = CAT_BOMB | 2,
    REMOTE_DETONATOR = CAT_BOMB | 3,
	HEALTH_PACK = CAT_BOMB | 4,

};
