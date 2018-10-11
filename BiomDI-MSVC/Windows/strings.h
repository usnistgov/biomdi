//most strings.h functionality is defined in string.h under Windows.

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

#define bcopy(s1,s2,n) memcpy(s1, s2, n);