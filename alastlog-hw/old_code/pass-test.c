#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>


int main()
{
	struct passwd *pp;

	pp = getpwent();

	while( pp )
	{
		printf("%s\t%d\n", pp->pw_name, pp->pw_uid);
		pp = getpwent();
	}

	endpwent();
	return 0;
}
