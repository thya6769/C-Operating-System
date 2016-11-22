#include <stdio.h>
#include <ctype.h>
 
int check_anagram(char [], char []);

void remove_punct_and_make_lower_case(char *p)
{
    char *src = p, *dst = p;

    while (*src)
    {
       if (ispunct((unsigned char)*src))
       {
          /* Skip this character */
          src++;
       }
       else if (isupper((unsigned char)*src))
       {
          /* Make it lowercase */
          *dst++ = tolower((unsigned char)*src);
          src++;
       }
       else if (src == dst)
       {
          /* Increment both pointers without copying */
          src++;
          dst++;
       }
       else
       {
          /* Copy character */
          *dst++ = *src++;
       }
    }

    *dst = 0;
} 
int main()
{
   char line[100], anagram[100];
   int flag;
 
   printf("Enter line: ");
   fgets(line, 100, stdin);
	
   printf("Enter anagram: ");
   fgets(anagram, 100, stdin);
	
	remove_punct_and_make_lower_case(line);
	remove_punct_and_make_lower_case(anagram);

  printf("\n");
   if(check_anagram(line, anagram) == 1) {
	   printf("Anagram!\n");
   } else {
	  printf("Not an anagram.\n");
   }
   return 0;
}
 
int check_anagram(char a[], char b[])
{
   int first[26] = {0}, second[26] = {0}, c = 0;
 
   while (a[c] != '\0')
   {
      first[a[c]-'a']++;
      c++;
   }
 
   c = 0;
 
   while (b[c] != '\0')
   {
      second[b[c]-'a']++;
      c++;
   }
 
   for (c = 0; c < 26; c++)
   {
      if (first[c] != second[c])
         return 0;
   }
 
   return 1;
}