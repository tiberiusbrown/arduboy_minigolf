#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include <vector>

struct vert { int8_t x, y, z; };
struct face { uint8_t i0, i1, i2, pt; };

int main()
{
    FILE* f = fopen("E:\\Projects\\arduboy_minigolf\\levels\\level_00.obj", "r");
    if(!f)
        return 1;

    std::vector<vert> verts;
    std::vector<face> faces;
    while(!feof(f))
    {
        int c = fgetc(f);
        if(c == 'v')
        {
            float x = 0;
            float y = 0;
            float z = 0;
            int n = fscanf(f, " %f %f %f\n", &x, &y, &z);
            if(n != 3)
                goto fail;
            int8_t ix = int8_t(roundf(x));
            int8_t iy = int8_t(roundf(y));
            int8_t iz = int8_t(roundf(z));
            verts.push_back({ ix, iy, iz });
        }
        else if(c == 'f')
        {
            int i0 = 0;
            int i1 = 0;
            int i2 = 0;
            int n = fscanf(f, " %d %d %d\n", &i0, &i1, &i2);
            if(n != 3)
                goto fail;
            uint8_t pt = 0;
            auto const& v0 = verts[i0];
            auto const& v1 = verts[i1];
            auto const& v2 = verts[i2];
            if(v0.x == v1.x && v1.x == v2.x) pt = 1;
            if(v0.y == v1.y && v1.y == v2.y) pt = 2;
            if(v0.z == v1.z && v1.z == v2.z) pt = 3;
            faces.push_back({ uint8_t(i0), uint8_t(i1), uint8_t(i2), pt });
        }
        else
        {
            while(fgetc(f) != '\n')
                ;
        }
    }

    fclose(f);

    f = fopen("E:\\Projects\\arduboy_minigolf\\levels\\level_00.hpp", "w");
    fprintf(f, "#pragma once\n\n");
    fprintf(f, "static constexpr int8_t LEVEL_00_VERTS[%d] PROGMEM =\n{\n",
        (int)verts.size() * 3);
    for(size_t i = 0; i < verts.size(); ++i)
    {
        fprintf(f, "%s%+4d, %+4d, %+4d,%c",
            i % 4 == 0 ? "    " : "",
            (int)verts[i].x, (int)verts[i].y, (int)verts[i].z,
            i % 4 == 3 ? '\n' : ' ');
    }
    fprintf(f, "\n};\n\n");
    fprintf(f, "static constexpr uint8_t LEVEL_00_FACES[%d] PROGMEM =\n{\n",
        (int)faces.size() * 4);
    for(size_t i = 0; i < faces.size(); ++i)
    {
        fprintf(f, "%s%3d, %3d, %3d, %3d,%c",
            i % 3 == 0 ? "    " : "",
            (int)faces[i].i0, (int)faces[i].i1, (int)faces[i].i2, (int)faces[i].pt,
            i % 3 == 2 ? '\n' : ' ');
    }
    fprintf(f, "\n};\n");

    fclose(f);

    return 0;

fail:
    fprintf(stderr, "Failed to convert obj\n");
    fclose(f);
    return 1;
}
