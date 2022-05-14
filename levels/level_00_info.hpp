#pragma once

static constexpr phys_box LEVEL_00_BOXES[] PROGMEM =
{
    make_box   (3  , 1  , 7  , 15  , -1,  3      ), // starting platform #1
    make_box   (6.5, 1  , 3  ,  5.5, -1,  7      ), // starting platform #2
    make_box_45(1.5, 1  , 0.5, 16.5,  0,  8.5, 32), // first wall
    make_box   (1.5, 2  , 1.5,  5.5,  0,  2.5    ), // tunnel left side
    make_box   (1.5, 2  , 1.5,  5.5,  0, 11.5    ), // tunnel right side
    //make_box   (2.25, 2.25, 3, -4, -1, 7, 0, 32), // slope
};
