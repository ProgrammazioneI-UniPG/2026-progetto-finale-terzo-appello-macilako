#ifndef GAMELIB_H
#define GAMELIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

//MACRO E COSTANTI DI SISTEMA
#define LUNGHEZZA_BUFFER_NOME 64
#define DIMENSIONE_ZAINO_GIOCATORE 3
#define NUMERO_TOTALE_ZONE_MAPPA 15
#define SOGLIA_PROBABILITA_NEMICO 30
#define SOGLIA_PROBABILITA_OGGETTO 40
#define NUMERO_VINCITORI_MEMORIZZATI 3



//Enun Tipo_zona rappresenta i diversi ambienti in cui i giocatori possono trovarsi.
typedef enum {
    bosco,
    scuola,
    laboratorio,
    caverna,
    strada,
    giardino,
    supermercato,
    centrale_elettrica,
    deposito_abbandonato,
    stazione_polizia
} Tipo_zona;


//Enum Tipo_nemico identifica le entità ostili presenti nel gioco.
typedef enum {
    nessun_nemico,
    billi,         
    democane,       
    demotorzone     
} Tipo_nemico;


//Enum Tipo_oggetto rappresenta gli oggetti raccoglibili per potenziare le statistiche.

typedef enum {
    nessun_oggetto,
    bicicletta,             // Oggetto per la fuga/fortuna
    maglietta_fuocoinferno, // Oggetto per l'attacco
    bussola,                // Oggetto generico
    schitarrata_metallica   // Oggetto per la difesa
} Tipo_oggetto;


struct Zona_soprasotto; 
struct Zona_mondoreale;

//DEFINIZIONE DELLE STRUTTURE DATI


typedef struct Zona_mondoreale {
    Tipo_zona tipo;                 
    Tipo_nemico nemico;            
    Tipo_oggetto oggetto;           
    
    struct Zona_mondoreale* avanti;    
    struct Zona_mondoreale* indietro;   
    struct Zona_soprasotto* link_soprasotto; 
} Zona_mondoreale;


typedef struct Zona_soprasotto {
    Tipo_zona tipo;                 
    Tipo_nemico nemico;            
    
    struct Zona_soprasotto* avanti;     
    struct Zona_soprasotto* indietro;   
    struct Zona_mondoreale* link_mondoreale; 
} Zona_soprasotto;


typedef struct Giocatore {
    char nome[LUNGHEZZA_BUFFER_NOME];
    int mondo; // Flag di stato
    

    struct Zona_mondoreale* pos_mondoreale;
    struct Zona_soprasotto* pos_soprasotto;
    
    int attacco_pischico;
    int difesa_pischica;
    int fortuna;
    

    Tipo_oggetto zaino[DIMENSIONE_ZAINO_GIOCATORE];
    int oggetti_posseduti;
    
    bool vivo; // Flag vita
} Giocatore;



void imposta_gioco();   
void gioca();           
void termina_gioco();   
void crediti();         

#endif 