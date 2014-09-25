#include "num2words-es.h"
#include "string.h"

static const char* STR_AND = "y ";
static const char* STR_ONE = "uno";
static const char* STR_TWENTY = "veinti";
static const char* STR_EMPTY = "        ";


static const char* const ONES[] = {
  "en punto",
  "una",
  "dos",
  "tres",
  "cuatro",
  "cinco",
  "seis",
  "siete",
  "ocho",
  "nueve"
};

static const char* const TEENS[] ={
  "",
  "once",
  "doce",
  "trece",
  "catorce",
  "quince",
  "dieci-seis",
  "dieci-siete",
  "dieci-ocho",
  "dieci-nueve"
};

static const char* const TENS[] = {
  "",
  "diez",
  "veinte",
  "treinta",
  "cuarenta",
  "cincuenta",
  "sesenta",
  "setenta",
  "ochenta",
  "noventa"
};

static size_t append_number(char* words, int num, int type) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {

    // caso de en punto
    if (tens_val == 1 && num != 10) 
    {

      // hacer append de "Y"
      if((num >= 10 && num <= 13 && type == 2))
      {
        strcat(words, STR_AND);
        len+= strlen(STR_AND);
      }
      
      strcat(words, TEENS[ones_val]);
      return strlen(TEENS[ones_val]);
    }
    

    // caso del veinti
    if(tens_val == 2 && type == 2 && num != 20)
    {
      strcat(words, STR_TWENTY);
      len+= strlen(STR_TWENTY);
    }
    else
    {

      // hacer append de "Y"
      if(((num == 10 || num == 20) && type == 2))
      {
        strcat(words, STR_AND);
        len+= strlen(STR_AND);
      }
      // caso normal
      strcat(words, TENS[tens_val]);
      len += strlen(TENS[tens_val]);
    }

    if (ones_val > 0) {
      strcat(words, " ");
      len += 1;
    }
  }

  if (ones_val > 0 || num == 0) 
  {
    switch(type)
    {
      case 1:    // hours

          strcat(words, ONES[ones_val]);
          len += strlen(ONES[ones_val]);
          break;

      case 2:     // minutes

          // hacer append de "Y" solo si cumple esta condicion
          if((num <= 20 || num > 29) && num != 0)
          {
            strcat(words, STR_AND);
            len+= strlen(STR_AND);
          }

          if(ones_val == 1)
          {
            strcat(words, STR_ONE);
            len += strlen(STR_ONE);
          }
          else
          {
            strcat(words, ONES[ones_val]);
            len += strlen(ONES[ones_val]);
          
          }

          break;
    }
    
  }
  return len;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}


void time_to_words(int hours, int minutes, char* words, size_t length) {

  size_t remaining = length;
  memset(words, 0, length);

  if (hours == 0 || hours == 12) {
    remaining -= append_string(words, remaining, TEENS[2]);
  } else {
    remaining -= append_number(words, hours % 12,1);
  }

  remaining -= append_string(words, remaining, " ");
  remaining -= append_number(words, minutes, 2);
  remaining -= append_string(words, remaining, " ");
}

int time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length)
{
	char value[length];
	time_to_words(hours, minutes, value, length);
	int flgReduceFont = 0;
  int flgFinish = 0;

	memset(line1, 0, length);
	memset(line2, 0, length);
	memset(line3, 0, length);
	
	char *start = value;
	char *pch = strstr (start, " ");

	while (pch != NULL) 
  {
		if (line1[0] == 0) 
    {
			memcpy(line1, start, pch-start);
		}  
    else if (line2[0] == 0) {
         memcpy(line2, start, pch-start);
        
        
        if(strlen(start) <= 9) {
        memcpy(line2, start, strlen(start) -1);
        int l = strlen(start);
        pch += l -1;

        }
      
		} else if (line3[0] == 0) {
			memcpy(line3, start, strlen(start)-1);
		}

		start += pch-start+1;
		pch = strstr(start, " ");
    
	}
	
	// Truncar valores largos de numeros TEEN
	if (strlen(line2) > 8) {

    // manejar font para reducirlo
    flgReduceFont = 1;

		char *pch2 = strstr(line2, "-");
		if (pch2) {
			memcpy(line3, pch2+1, strlen(line2)-5);
			pch2[0] = 0;
      flgReduceFont = 0;
		}
    
    
	}

  return flgReduceFont;
}