#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>


int main()
{
	struct passwd *pp;

	pp = getpwent();

	while( pp )
	{
		printf("%s\t%s\n", pp->pw_name, pp->pw_gecos);
		pp = getpwent();
	}

	endpwent();
	return 0;
}
