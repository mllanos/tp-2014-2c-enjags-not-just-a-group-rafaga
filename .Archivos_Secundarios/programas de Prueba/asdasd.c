#include <stdio.h>
#include <stdlib.h>

main() {

	char a[130],b[130];

	scanf("%*[ \t]%[^\n]",b);

	printf("B: %s\n",b);

	//scanf("%[ \t]%[^\n]",a,b);

	//printf("A: %s\nB: %s\n",a,b);

}
