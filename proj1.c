/***************************************************************/
/***                        Projekt 1                        ***/
/***                     Textovy editor                      ***/
/***                                                         ***/
/***               Dominik Nejedly (xnejed09)                ***/
/***                          2018                           ***/
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POCET_PRIKAZU 100
#define DELKA_RADKU 1252
#define DELKA_PRIKAZU 1252


int nacitani_prikazu(char *soubor_s_prikazy, char (*prikazy)[DELKA_PRIKAZU], int *pocet_prikazu);
int vyber_prikazu(char *prikaz, int *cislo_prikazu, char *text, int *pomocna_promenna, int pocet_prikazu, char (*prikazy)[DELKA_PRIKAZU]);
void smazat(int pocet_opak, char *text, int *pomocna_promenna);
void cteni_radku(char *text);
int preved(char *prikaz);
void vypis(int pocet_opak, char *text, int *pomocna_promenna);
int skok(int cislo, int *cislo_prikazu, int pocet_prikazu, char (*prikazy)[DELKA_PRIKAZU]);
void smazat_EOL(char *radek);
void vlozit_radek(char *prikaz);
int text_zacatek(char *prikaz, char *text);
int text_konec(char *prikaz, char *text);
int substituce_s(char *prikaz, char *text);
int substituce_S(char *prikaz, char *text);
void podmineny_vypis(char *prikaz, char *text, int *pomocna_promenna);
int konci_na_EOL(char *text);
void chybove_hlaseni(int navratova_hodnota, int aktualni_prikaz, char (*prikazy)[DELKA_PRIKAZU]); 


int main(int argc, char *argv[])
{
	char prikazy[POCET_PRIKAZU][DELKA_PRIKAZU];		// pole pro nacteni vsech prikazu
	char text_radek[DELKA_RADKU];					// pole pro načitani vstupu
	int pocet_prikazu;								// celkovy pocet prikazu v souboru s prikazy
	int aktualni_prikaz;							// cislo aktualniho prikazu
	int pomocna_promenna = 1;						// kontrola znaku konce radku
	int navratova_hodnota;							// promenna pro ukladani návratove hodnoty funkce vyber_prikazu

	// kontrola poctu argumentu
	if(argc != 2)
	{
		printf("Textovy editor, ktery cte vstupni data ze standardniho vstupu.\n"
		       "Ta jsou upravovana pomoci prikazu ze souboru a tisknuta na standardni vystup.\n"
		       "Pred spustenim zadejte jako 1 argument a to textovy soubor s prikazy.\n"
		       "Pro opetovne vypsani napovedy zadejte pred spustenim programu vice argumentu nebo nezadavejte zadny.\n");

		return 0;
	}

	// Pri uspesnem nacteni souboru zacina uprava.
	if(nacitani_prikazu(argv[1], prikazy, &pocet_prikazu) == 1)
	{
		cteni_radku(text_radek);
		
		for(aktualni_prikaz = 0; aktualni_prikaz < pocet_prikazu; aktualni_prikaz++)
		{
			// Pokud je aktualnim prikazem prikaz q nebo jiz nejsou data na vstupu, program uspesne skonci.
			if(prikazy[aktualni_prikaz][0] == 'q' || feof(stdin))
			{
				break;
			}

			// ulozeni navratove hodnoty funkce
			navratova_hodnota = vyber_prikazu(prikazy[aktualni_prikaz], &aktualni_prikaz, text_radek, &pomocna_promenna, pocet_prikazu, prikazy);

			// kontrola navratove hodnoty
			if(navratova_hodnota > 0)
			{
				chybove_hlaseni(navratova_hodnota, aktualni_prikaz, prikazy);
				return navratova_hodnota;
			}
		}
		
		if(prikazy[aktualni_prikaz][0] != 'q')
		{
			// Pokud dojdou prikazy, vypise se zbytek vstupniho souboru.
			while(!feof(stdin))
			{
				vypis(1, text_radek, &pomocna_promenna);
			}
		}
		else
		{
			// Pokud je aktualnim prikazem q a nachazi se pred nim nejaky prikaz upravujici text, vytiskne aktualni radek.
			if((!feof(stdin)) && aktualni_prikaz != 0 && (prikazy[aktualni_prikaz - 1][0] == 'a' || prikazy[aktualni_prikaz - 1][0] == 'b' || prikazy[aktualni_prikaz - 1][0] == 's' || prikazy[aktualni_prikaz - 1][0] == 'S'))
			{
				if(navratova_hodnota == 0)
				{
					vypis(1, text_radek, &pomocna_promenna);
				}
			}
		}

		// Pokud posledni radek na vstupu neobsahuje znak konce řádku, odradkuje.
		if(pomocna_promenna == 0)
		{
			printf("\n");
		}
	}

	return 0;
}


/**
 * Funkce načita prikazy ze souboru do dvourozmerneho pole prikazy a vraci 1, pokud skonci uspesne. V opacnem pripade vrati 0.
 */
int nacitani_prikazu(char *soubor_s_prikazy, char (*prikazy)[DELKA_PRIKAZU], int *pocet_prikazu)
{
	*pocet_prikazu = 0;
	FILE *soubor;
	soubor = fopen(soubor_s_prikazy, "r");

	if(soubor != NULL)
	{
		// Nacte prikazy a u kazdeho odstrani znak konce radku.
		while(fgets(prikazy[*pocet_prikazu], DELKA_PRIKAZU, soubor) != NULL && *pocet_prikazu < POCET_PRIKAZU)
		{
			smazat_EOL(prikazy[*pocet_prikazu]);
			(*pocet_prikazu)++;
		}

		fclose(soubor);
		
		if(*pocet_prikazu == POCET_PRIKAZU)
		{
			fprintf(stderr, "Nepodporovane mnozstvi prikazu!\n");
			return 0;
		}

		return 1;
	}

	fprintf(stderr, "Soubor s prikazy %s se nepodarilo otevrit.\n", soubor_s_prikazy);
	return 0;
}


/**
 * Funkce rozhoduje, o jaky prikaz ze souboru s prikazy se jedna, a pote vola funkce danych prikazu.
 */
int vyber_prikazu(char *prikaz, int *cislo_prikazu, char *text, int *pomocna_promenna, int pocet_prikazu, char (*prikazy)[DELKA_PRIKAZU])
{
	int cislo;
	int navratova_hodnota;

	switch(prikaz[0])
	{
		case 'd':
			cislo = preved(prikaz);
			smazat(cislo, text, pomocna_promenna); 	
			break;
		case 'n':
			cislo = preved(prikaz);
			vypis(cislo, text, pomocna_promenna);
			break;
		case 'g':
			cislo = preved(prikaz);
			navratova_hodnota = skok(cislo, cislo_prikazu, pocet_prikazu, prikazy);
			return navratova_hodnota;
		case 'r':
			smazat_EOL(text);
			break;
		case 'i':
			vlozit_radek(prikaz);
			break;
		case 'b':
			navratova_hodnota = text_zacatek(prikaz, text);
			return navratova_hodnota;
		case 'a':
			navratova_hodnota = text_konec(prikaz, text);
			return navratova_hodnota;
		case 's':
			navratova_hodnota = substituce_s(prikaz, text);
			return navratova_hodnota;
		case 'S':
			navratova_hodnota = substituce_S(prikaz, text);
			return navratova_hodnota;
		case 'f':
			podmineny_vypis(prikaz, text, pomocna_promenna);
			break;
		default:
			return 6;
	}

	return 0;
}


/**
 * Funkce dle navratove hodnoty jine funkce vybira chybove hlaseni a tiskne je.
 */
void chybove_hlaseni(int navratova_hodnota, int aktualni_prikaz, char (*prikazy)[DELKA_PRIKAZU])
{
	switch(navratova_hodnota)
	{
		case 1:
			fprintf(stderr, "Soubor s prikazy je zacykleny! Problem se nachazi na %d. radku.\n", aktualni_prikaz + 1);
			break;
		case 2:
			fprintf(stderr, "Na %d. radku v souboru s prikazy se nachazi skok na neexistujici prikaz!\n", aktualni_prikaz + 1);
			break;
		case 3:
			fprintf(stderr, "Na %d. radku v souboru s prikazy neni uvedeno cislo radku pro skok!\n", aktualni_prikaz + 1);
			break;
		case 4:
			fprintf(stderr, "Pri zpracovani příkazu '%c' na %d. radku v souboru s prikazy, dochazi k prekroceni maximalniho poctu znaku na radku.\n", prikazy[aktualni_prikaz][0], aktualni_prikaz + 1);
			break;
		case 5:
			fprintf(stderr, "U prikazu substituce '%c' na %d. radku v souboru s prikazy, neni zadan hledany vzor!\n", prikazy[aktualni_prikaz][0], aktualni_prikaz + 1);
			break;
		case 6:
			fprintf(stderr, "Na %d. radku v souboru s prikazy se nachazi neznamy prikaz '%c'!\n", aktualni_prikaz + 1, prikazy[aktualni_prikaz][0]);
			break;
	}
}


/**
 * Funkce maze aktualni radek. Ve skutecnosti ho vsak jen nevytiskne a nacte dalsi.
 * Pocet opakovani zavisi na cisle za prikazem v souboru s prikazy.
 */
void smazat(int pocet_opak, char *text, int *pomocna_promenna)
{
	*pomocna_promenna = konci_na_EOL(text);

	if(pocet_opak == -1)
	{
		pocet_opak = 1;
	}

	for(int i = 0; i < pocet_opak && (!feof(stdin)); i++)
	{
		cteni_radku(text);
	}
}


/**
 * Funkce, ktera nacte dalsi radek.
 */
void cteni_radku(char *text)
{
	fgets(text, DELKA_RADKU, stdin);
}


/**
 * Funkce prevadi znaky cisel, ktera se nachazi v retezcich prikazu za jejich nazvy, na datovy typ celych cisel.
 * Navratovou hodnotou je prevedene cislo, -1 (v pripade, ze se za nazvem prikazu nachazi znak konce retezce), nebo 0.
 */
int preved(char *prikaz)
{
	int cislo = 0;

	if(prikaz[1] == '\0')
	{
		return -1;
	}
	
	for(int i = 1; prikaz[i] != '\0' && prikaz[i] >= '0' && prikaz[i] <= '9'; i++)
	{
		cislo = (cislo * 10) + (prikaz[i] - '0');
	}

	return cislo;
}


/**
 * Funkce vypise radek ze vstupniho souboru na standardni vystup a nacte dalsi.
 * Pocet opakovani zavisi na cisle za prikazem v souboru.
 */
void vypis(int pocet_opak, char *text, int *pomocna_promenna)
{
	*pomocna_promenna = konci_na_EOL(text);
	
	if(pocet_opak == -1)
	{
		pocet_opak = 1;
	}

	for(int i = 0; i < pocet_opak && (!feof(stdin)); i++)
	{
		printf("%s", text);
		cteni_radku(text);
	}
}


/**
 * Funkce zjisti, zda dany radek konci znakem odradkovani a vrati 1, pokud ano.
 */
int konci_na_EOL(char *radek)
{
	int delka_radku = strlen(radek);
	
	if(radek[delka_radku - 1] == '\n')
	{
		return 1;
	}
	
	return 0;
}


/**
 * Funkce odstrani znak odradkovani.
 */
void smazat_EOL(char *radek)
{
	if(konci_na_EOL(radek) == 1)
	{
		radek[strlen(radek) - 1] = '\0';
	}	
}


/**
 * Funkce skoci z aktualniho radku v souboru s prikazy na radek, ktery ma cislo nachazejici se za názvem tohoto prikazu v souboru s prikazy.
 */
int skok(int cislo, int *cislo_prikazu, int pocet_prikazu, char (*prikazy)[DELKA_PRIKAZU])
{
	if(cislo > 0 && cislo <= pocet_prikazu)
	{
		for(int i = cislo - 1; i < *cislo_prikazu; i++)
		{	
			if(prikazy[i][0] == 'n' || prikazy[i][0] == 'd' || prikazy[i][0] == 'f')
			{
				*cislo_prikazu = cislo - 2;
				return 0;
			}
		}

		return 1;
	}
	else if(cislo > pocet_prikazu)
	{
		return 2;
	}
	else
	{
		return 3;
	}
}


/**
 * Funkce vlozi pred aktualni radek text, ktery se nachazi v souboru s prikazy za nazvem tohoto prikazu.
 */
void vlozit_radek(char *prikaz)
{
	int delka_prikazu = strlen(prikaz);
	char pomocne_pole[DELKA_PRIKAZU];	// pro ulozeni textu za prikazem

	for(int i = 1; i <= delka_prikazu; i++)
	{
		pomocne_pole[i - 1] = prikaz[i];
	}
	
	printf("%s\n", pomocne_pole);
}


/**
 * Funkce vlozi pred zacatek aktualniho radku text, ktery se nachazi v souboru s prikazy za nazvem tohoto prikazu.
 */
int text_zacatek(char *prikaz, char *text)
{
	int delka_radku = strlen(text);
	int delka_prikazu = strlen(prikaz);

	// kontrola poctu znaku
	if(delka_prikazu > 1)
	{
		if((delka_radku + delka_prikazu - 1) < DELKA_RADKU)
		{
			// vytvoreni mista (posunuti vsech znaku)
			for(int i = delka_radku; i >= 0; i--)
			{
				text[i + delka_prikazu - 1] = text[i];
			}

			// vlozeni textu
			for(int j = 1; j < delka_prikazu; j++)
			{
				text[j - 1] = prikaz[j];
			}

			return 0;
		}

		return 4;
	}

	return -1;
}


/**
 * Funkce vlozi na konec aktualniho radku text, ktery se nachazi v souboru s prikazy za nazvem tohoto prikazu.
 */
int text_konec(char *prikaz, char *text)
{
	int delka_radku = strlen(text);
	int delka_prikazu = strlen(prikaz);

	// kontrola poctu znaku
	if(delka_prikazu > 1)
	{
		if((delka_radku + delka_prikazu - 1) < DELKA_RADKU)
		{	
			// zjisteni, zda konci znakem konce radku
			int odradkovat = konci_na_EOL(text);
			smazat_EOL(text);

			// pridani textu
			for(int i = 1; i < delka_prikazu; i++)
			{
				text[delka_radku - odradkovat + i - 1] = prikaz[i];
			}
			// pridani znaku konce retezce popr. konce řádku
			if(odradkovat == 1)
			{
				text[delka_radku - odradkovat + delka_prikazu - 1] = '\n';
				text[delka_radku - odradkovat + delka_prikazu] = '\0';
			}
			else
			{
				text[delka_radku - odradkovat + delka_prikazu - 1] = '\0';
			}

			return 0;
		}

		return 4;
	}

	return -1;
}


/**
 * Funkce zjisti delku retezce, ktery se ma nahradit.
 */
int zjisti_delku_vzoru(char *prikaz)
{
	int delka_vzoru = 0;

	for(int i = 2; prikaz[i] != prikaz[1]; i++)
	{
		delka_vzoru++;
	}

	return delka_vzoru;
}


/**
 * Funkce zjisti delku retezce, ktery nahrazuje vzor.
 */
int zjisti_delku_nahrady(int delka_vzoru, char *prikaz)
{
	int delka_nahrady = 0;

	for(int i = delka_vzoru + 3; prikaz[i] != '\0'; i++)
	{
		delka_nahrady++;
	}

	return delka_nahrady;
}


/**
 * Funkce, která hleda dany vzor na aktualnim radku souboru na standardnim vstupu.
 * Parametr odsazeni udava pocatecni index vzoru a koncovy znak prvni znak za koncem vzoru v souboru s prikazy.
 * Vraci pocatecni index vzoru, nebo -1, pokud se vzor na danem radku nenachazi.
 */
int najdi_vzor(int odsazeni, char koncovy_znak, char *prikaz, char *text)
{
	int delka_vzoru = zjisti_delku_vzoru(prikaz);
	
	if(delka_vzoru != 0)
	{
		// Nacita jednotlive znaky na radku.
		for(int i = 0; text[i] != '\0'; i++)
		{
			// Hleda shodu.
			for(int j = 0; (prikaz[j + odsazeni] != '\0') && (text[i + j] != '\0') && (text[i + j] == prikaz[j + odsazeni]) && (prikaz[j + odsazeni] != koncovy_znak); j++)
			{
				if(prikaz[j + odsazeni + 1] == koncovy_znak)
				{
					return i;
				}
			}
		}

		return -1;
	}

	return -2;
}


/**
 * Funkce vymeni vzor za nahradu.
 */
int nahrazeni(int pocatecni_index, char *prikaz, char *text)
{
	int delka_vzoru = zjisti_delku_vzoru(prikaz);
	int delka_nahrady = zjisti_delku_nahrady(delka_vzoru, prikaz);
	int delka_radku = strlen(text);
	
	// kontrola poctu znaku
	if((delka_radku - delka_vzoru + delka_nahrady) < DELKA_RADKU)
	{
		// smazani nalezeneho vzoru
		for(int i = pocatecni_index + delka_vzoru; i <= delka_radku; i++)
		{
			text[i - delka_vzoru] = text[i];
		}

		// vytvoreni mista pro retezec nahrazeni
		for(int i = delka_radku; i >= pocatecni_index; i--)
		{
			text[i + delka_nahrady] = text[i];
		}

		// vlozeni retezce nahrazeni
		for(int i = 0; i < delka_nahrady; i++)
		{			
			text[pocatecni_index + i] = prikaz[delka_vzoru + i + 3];
		}

		return 0;
	}

	return 4;
}


/**
 * Funkce vymeni prvni vyskyt hledaneho vzoru na radku za retezec, kterym se ma nahradit.
 */
int substituce_s(char *prikaz, char *text)
{
	int navratova_hodnota = -1;
	int pocatecni_index = najdi_vzor(2, prikaz[1], prikaz, text);
	
	if(pocatecni_index == -2)
	{
		return 5;
	}
	else if(pocatecni_index != -1)
	{
		navratova_hodnota = nahrazeni(pocatecni_index, prikaz, text);
	}

	return navratova_hodnota;
}


/**
 * Funkce vymeni kazdy vykyt hledaneho vzoru na radku za retezec, kterym se ma nahradit.
 */
int substituce_S(char *prikaz, char *text)
{
	int navratova_hodnota = 0;
	int pocatecni_index;

	do
	{
		pocatecni_index = najdi_vzor(2, prikaz[1], prikaz, text);
		
		if(pocatecni_index == -2)
		{
			return 5;
		}
		else if(pocatecni_index != -1 && navratova_hodnota == 0)
		{
			navratova_hodnota = nahrazeni(pocatecni_index, prikaz, text);
		}
	}while(pocatecni_index != -1 && navratova_hodnota == 0);
	
	return navratova_hodnota;
}


/**
 * Funkce tiske a nacita dalsi radky, dokud na nekterem z nich nenalezne hledany vzor, nebo nevytiskne cely vstup.
 */
void podmineny_vypis(char *prikaz, char *text, int *pomocna_promenna)
{
	int pocatecni_index;
	
	do
	{
		pocatecni_index = najdi_vzor(1, '\0', prikaz, text);
		
		if(pocatecni_index == -1)
		{
			vypis(1, text, pomocna_promenna);
		}
	}while(pocatecni_index == -1);
}