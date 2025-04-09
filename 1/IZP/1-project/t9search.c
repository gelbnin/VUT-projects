#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void switchNumbers(char table[8][8],char name_contact[], char number_buffer[])
// funkce hleda znak z pracovniho retezce name_contakt, a ulkada odpovidajici tomu znaku cislo na stejne misto do pomocniho retezce name_buffer, z jehoz pomoci probehne vychledavani
{
	int s3 = strlen(name_contact);

	for(int k = 0; k <= s3; k++)
	{
		if((name_contact[k] >= 'A' && name_contact[k] <= 'Z') || (name_contact[k] >= 'a' && name_contact[k] <= 'z'))
		{
			for(int j = 0; j <= 7; j++)
			{
			for(int i = 0; i <= 7; i++)
			{
			if(name_contact[k] == table[j][i])
				{
				number_buffer[k] = j + '2';
				j = 7;
				i = 7;
				}
			}
			}
		}
		else if(name_contact[k] >= '0' && name_contact[k] <= '9')
			number_buffer[k] = name_contact[k];
		else if(name_contact[k] == '+')
			number_buffer[k] = '0';
		else
			number_buffer[k] = ' ';
	}
	return;
}


void scanNumber_100limiter(char number_buffer[], int limitDtc)
// funkce skenuje druhy retezec jedneho kola ciklu jemuz odpovida cislo, a taky omezuje pocet znaku z kazdeho retezce ze stdin
{
	if(limitDtc == 100)
		scanf("%*[^\n]\n");
		// omezeni poctu znaku probiha formatovanim zbytku retezce, podminkou pro co je vraceni 100 poctu naskenovanych znaku
	scanf("%100[^\n]%n\n", number_buffer, &limitDtc);

	if(limitDtc == 100)
		scanf("%*[^\n]\n");
	return;
}


void concatenation(char name_contact[], char number_buffer[])
// funcke spojuje retezce se jmenem a cislem,a uklada vsechno do name_contact, pro dalsi praci s jednim retezcem
{
	int s1 = strlen(name_contact);
	int s2 = strlen(number_buffer);

	name_contact[s1 - 1] = ',';
	name_contact[s1] = ' ';

	for(int j = 0; j <= s2; j++)
		name_contact[j + (s1 + 1)] = number_buffer[j];
	return;
}


int main(int argc, char *argv[])
{
	char name_contact [202];			// retezec pro zapsani jmena kontakta a vydavani hledaneho kontaktu
	char number_buffer [202];			// retezec pro cislo kontakta a pomocni buffer
	char table[8][8] = {"abcABC", "defDEF", "ghiGHI", "jklJKL", "mnoMNO", "pqrsPQRS", "tuvTUV", "wxyzWXYZ"}; // pomocni retezec pro prepsani pismen kontaktu na odpovidajici cisla
	int findCount = 0;
	int limitDtc = 0;

	//chybova hlaseni spatne zvoleneho argumentu
	if(argc > 2) 
	{
		fprintf(stderr, "Enter only 1 argument!\n");
		return 1;
	}
	else if(argc != 1 && (strspn(argv[1], " 0123456789") != (strlen(argv[1]))))
	{
		fprintf(stderr, "Enter only numbers in the argument!\n");
		return 1;
	}

	// vydava vsechny vysledky pokud nebyl zadan zadny argument
	else if(argc == 1)
	{
		while(scanf("%100[^\n] %n \n", name_contact, &limitDtc) != EOF)
		// hlavni ciklus v jednem kole ktereho probiha skenovani dvou retezcu ze stdin
		{
			scanNumber_100limiter(number_buffer, limitDtc);
			concatenation(name_contact, number_buffer);
			printf("%s\n", name_contact);
		}
	}

	// hledani pri spravne zvolenem argumentu
	else
	{
		while(scanf("%100[^\n] %n \n", name_contact, &limitDtc) != EOF)	
		{
			scanNumber_100limiter(number_buffer, limitDtc);
			concatenation(name_contact, number_buffer);
			switchNumbers(table, name_contact, number_buffer);

			// printuje pracovni retezec cyklu pokud najde shodu argumentu z pomocnim retezcem
			if(strstr(number_buffer, argv[1]) != NULL)
			{
				printf("%s\n", name_contact);
				findCount++;
			}
			// nuluje pomocni retezec aby zabranilo chyby hledani nasledujiciho ciklu
			memset(number_buffer, 0, strlen(number_buffer));
		}
	}

	//hlaseni pokud nebyl nalezen zadny kontakt
	if(findCount == 0 && argc != 1)
	{
		printf("Not found\n");
		return 0;
	}
    return 0;
}
