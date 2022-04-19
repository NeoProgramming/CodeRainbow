#pragma once

struct Loc {
    int row;
    int col;
    int pos;

	Loc() 
	{
        pos = -1;
        row = -1;    // invalidate
        col = 0;
	}
    bool valid()
    {
        return row >= 0;
    }
    bool operator==(const Loc &loc)
    {
        return row==loc.row && col==loc.col;
    }
};
