char *remove_ext(char *mystr, char dot, char sep)
{
	char *retstr, *lastdot, *lastsep;

	if (!mystr) return 0;
	if (!(retstr = malloc(strlen(mystr)+1))) return 0;

	strcpy(retstr, mystr);
	lastdot = strrchr(retstr, dot);
	lastsep = (sep == 0) ? 0 : strrchr(retstr, sep);

	if (lastdot) {
		if (lastsep) {
			if (lastsep < lastdot) {
				*lastdot = '\0';
			}
		} else {
			*lastdot = '\0';
		}
	}

	return retstr;
}

