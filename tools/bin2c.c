#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    // args: <infile> <outfile>
    if(argc < 3) return 1;
    
    FILE* fi = fopen(argv[1], "rb");
    if(!fi) return 1;
    FILE* fo = fopen(argv[2], "w");
    if(!fo)
    {
        fclose(fi);
        return 1;
    }
    
    fprintf(fo, "#pragma once\n\n");
    fprintf(fo, "static constexpr uint8_t FX_DATA[] PROGMEM =\n{\n    ");
    
    int n = 0;
    while(!feof(fi))
    {
        uint8_t d;
        fread(&d, 1, 1, fi);
        fprintf(fo, "%3d,%s", (int)d, n % 8 == 7 ? "\n    " : " ");
        ++n;
    }
    
    fprintf(fo, "\n};\n");
    
    fclose(fi);
    fclose(fo);
    
    return 0;
}
