#include "gamelib.h"
//GESTISCE IL MENU PRINCILAPE
int main() {
    // Inizializzazione del seme casuale basato sul tempo corrente
    srand((unsigned int)time(NULL));
    
    int scelta_utente = 0;
    
   
    printf("BENVENUTI A OCCHINZ\n");


    do {
        printf("\n--- MENU PRINCIPALE ---\n");
        printf("1) Imposta Nuova Partita\n");
        printf("2) Inizia Gioco\n");
        printf("3) Termina e Esci\n");
        printf("4) Visualizza Crediti / Vincitori\n");
        printf("Seleziona opzione ");
        
        // Lettura e validazione base dell'input
        if (scanf("%d", &scelta_utente) != 1) {
            printf("Errore: Inserire un numero valido.\n");
            // Svuota buffer tastiera
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            scelta_utente = 0; // Reset scelta per ripetere loop
            continue;
        }

        // Dispatch verso le funzioni di libreria
        switch (scelta_utente) {
            case 1:
                imposta_gioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                termina_gioco();
                break;
            case 4:
                crediti();
                break;
            default:
                printf("Opzione non riconosciuta. Scegliere tra 1 e 4.\n");
                break;
        }
        
    } while (scelta_utente != 3); // Esci solo se scelta è 3

    return 0;
}