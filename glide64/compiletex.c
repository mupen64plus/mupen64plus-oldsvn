#include <stdio.h>

int main(int argc, char **argv)
{
   FILE *src, *dest;
   unsigned char a;
   
   src = fopen(argv[1], "rb");
   dest = fopen(argv[2], "wb");
   
   fprintf(dest, "unsigned char %s[] = {", argv[3]);
   
   while(fread(&a, 1, 1, src))
     {
	fprintf(dest, "%d,\n", a);
     }
   fprintf(dest, "0};\n");
   fclose(src);
   fclose(dest);
   
   return 0;
}
