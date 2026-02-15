#include "gamelib.h"

// Gestione Giocatori
static struct Giocatore** elenco_giocatori_globali = NULL;
static int numero_giocatori_attuali = 0;

// Gestione Mappa 
static struct Zona_mondoreale* radice_mappa_reale = NULL;
static struct Zona_soprasotto* radice_mappa_soprasotto = NULL;

// Flag di Stato del Sistema
static bool flag_gioco_inizializzato = false;
static bool flag_mappa_costruita = false;
static bool flag_boss_generato = false;

// Vincitori
static char registro_vincitori[NUMERO_VINCITORI_MEMORIZZATI][LUNGHEZZA_BUFFER_NOME];
static int conteggio_vincitori_registrati = 0;


// Funzioni Unitili
static void pulisci_flusso_input(void);
static void attendi_input_utente(void);
static int genera_intero_casuale(int min, int max);
static int acquisisci_intero_sicuro(int min, int max);
static void stampa_intestazione_fase(const char* titolo);

// Conversioni e Stampe 
static const char* converti_enum_zona_stringa(Tipo_zona z);
static const char* converti_enum_nemico_stringa(Tipo_nemico n);
static const char* converti_enum_oggetto_stringa(Tipo_oggetto o);
static void stampa_dettagli_giocatore(struct Giocatore* g_ptr);
static void stampa_descrizione_luogo(struct Giocatore* g_ptr);

// Gestione Memoria
static void resetta_memoria_completa(void);
static void libera_memoria_giocatori(void);
static void libera_memoria_mappe(void);
static struct Zona_mondoreale* alloca_nuovo_nodo_reale(void);
static struct Zona_soprasotto* alloca_nuovo_nodo_soprasotto(void);

// Logica Mappa 
static void procedura_creazione_mappa(void);
static void collega_nodi_mappa_append(struct Zona_mondoreale* mr, struct Zona_soprasotto* ss);
static void inserisci_in_coda_reale(struct Zona_mondoreale* nuovo_nodo);
static void inserisci_in_coda_soprasotto(struct Zona_soprasotto* nuovo_nodo);
static void inserisci_zona_manuale(void);
static void cancella_zona_manuale(void);
static int calcola_lunghezza_mappa_attuale(void);
static void collega_nodi_in_posizione_specifica(struct Zona_mondoreale* n_mr, struct Zona_soprasotto* n_ss, int pos);
static void rimuovi_nodi_in_posizione_specifica(int pos);


static Tipo_zona acquisizione_manuale_tipo_zona(void);
static Tipo_nemico acquisizione_manuale_nemico_mr(void);
static Tipo_oggetto acquisizione_manuale_oggetto(void);

// Logica 
static void configura_singolo_giocatore(int indice);
static void esegui_turno_gioco(struct Giocatore* g_corr);
static void gestisci_azione_avanzamento(struct Giocatore* g);
static void gestisci_azione_arretramento(struct Giocatore* g);
static void gestisci_azione_cambio_dimensione(struct Giocatore* g);
static void gestisci_scontro_nemico(struct Giocatore* g);
static void gestisci_raccolta_risorse(struct Giocatore* g);
static void gestisci_utilizzo_inventario(struct Giocatore* g);

//Gestione Vincitori 
static void aggiorna_registro_vincitori(const char* nome_vincitore);


static void pulisci_flusso_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static void attendi_input_utente(void) {
    printf("\n[Premi INVIO per continuare...]");
    pulisci_flusso_input();
}

static int genera_intero_casuale(int min, int max) {
    return min + rand() % (max - min + 1);
}

static int acquisisci_intero_sicuro(int min, int max) {
    int valore_letto;
    int risultato_scanf;
    
    do {
        risultato_scanf = scanf("%d", &valore_letto);
        
        if (risultato_scanf != 1) {
            printf("Errore: Input non numerico. Riprova: ");
            pulisci_flusso_input();
        } else if (valore_letto < min || valore_letto > max) {
            printf("Errore: Valore fuori dal range [%d-%d]. Riprova: ", min, max);
            pulisci_flusso_input(); 
        } else {
            pulisci_flusso_input(); 
            return valore_letto;
        }
    } while (1); 
}

static void stampa_intestazione_fase(const char* titolo) {
    printf("\n");
    printf("   %s\n", titolo);
}


static void resetta_memoria_completa(void) {
    printf("Procedura di pulizia memoria...\n");
    libera_memoria_giocatori();
    libera_memoria_mappe();
    flag_gioco_inizializzato = false;
    flag_mappa_costruita = false;
    flag_boss_generato = false;
    printf("Memoria resettata con successo.\n");
}

static void libera_memoria_giocatori(void) {
    if (elenco_giocatori_globali != NULL) {
        for (int i = 0; i < numero_giocatori_attuali; i++) {
            if (elenco_giocatori_globali[i] != NULL) {
                free(elenco_giocatori_globali[i]);
                elenco_giocatori_globali[i] = NULL;
            }
        }
        free(elenco_giocatori_globali);
        elenco_giocatori_globali = NULL;
    }
    numero_giocatori_attuali = 0;
}

static void libera_memoria_mappe(void) {
    struct Zona_mondoreale* cursore_mr = radice_mappa_reale;
    while (cursore_mr != NULL) {
        struct Zona_mondoreale* temp = cursore_mr;
        cursore_mr = cursore_mr->avanti;
        free(temp);
    }
    radice_mappa_reale = NULL;

    struct Zona_soprasotto* cursore_ss = radice_mappa_soprasotto;
    while (cursore_ss != NULL) {
        struct Zona_soprasotto* temp = cursore_ss;
        cursore_ss = cursore_ss->avanti;
        free(temp);
    }
    radice_mappa_soprasotto = NULL;
}

static struct Zona_mondoreale* alloca_nuovo_nodo_reale(void) {
    struct Zona_mondoreale* nuovo_ptr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
    if (nuovo_ptr == NULL) {
        fprintf(stderr, "ERRORE CRITICO: Memoria esaurita durante allocazione zona MR.\n");
        exit(EXIT_FAILURE);
    }
    nuovo_ptr->avanti = NULL;
    nuovo_ptr->indietro = NULL;
    nuovo_ptr->link_soprasotto = NULL;
    nuovo_ptr->oggetto = nessun_oggetto;
    nuovo_ptr->nemico = nessun_nemico;
    return nuovo_ptr;
}

static struct Zona_soprasotto* alloca_nuovo_nodo_soprasotto(void) {
    struct Zona_soprasotto* nuovo_ptr = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));
    if (nuovo_ptr == NULL) {
        fprintf(stderr, "ERRORE CRITICO: Memoria esaurita durante allocazione zona SS.\n");
        exit(EXIT_FAILURE);
    }
    nuovo_ptr->avanti = NULL;
    nuovo_ptr->indietro = NULL;
    nuovo_ptr->link_mondoreale = NULL;
    nuovo_ptr->nemico = nessun_nemico;
    return nuovo_ptr;
}

static void procedura_creazione_mappa(void) {
    if (flag_mappa_costruita) {
        printf("Rilevata mappa preesistente. Cancellazione in corso...\n");
        libera_memoria_mappe();
    }

    printf("Generazione procedurale di %d zone in corso...\n", NUMERO_TOTALE_ZONE_MAPPA);
    flag_boss_generato = false; 

    for (int i = 0; i < NUMERO_TOTALE_ZONE_MAPPA; i++) {
        struct Zona_mondoreale* nodo_mr = alloca_nuovo_nodo_reale();
        struct Zona_soprasotto* nodo_ss = alloca_nuovo_nodo_soprasotto();

        // Assegnazione Tipo Zona
        Tipo_zona tipo_generato = (Tipo_zona) genera_intero_casuale(0, 9);
        nodo_mr->tipo = tipo_generato;
        nodo_ss->tipo = tipo_generato;

        // Popolazione MR
        if (genera_intero_casuale(1, 100) <= SOGLIA_PROBABILITA_NEMICO) {
            nodo_mr->nemico = billi;
        }
        if (genera_intero_casuale(1, 100) <= SOGLIA_PROBABILITA_OGGETTO) {
            nodo_mr->oggetto = (Tipo_oggetto) genera_intero_casuale(1, 4); 
        }

        // Popolazione SS
        if (genera_intero_casuale(1, 100) <= SOGLIA_PROBABILITA_NEMICO) {
            nodo_ss->nemico = democane;
        }

        // Logica Boss (Demotorzone)
        if (!flag_boss_generato) {
            bool forza_boss = (i == NUMERO_TOTALE_ZONE_MAPPA - 1);
            bool chance_boss = (genera_intero_casuale(1, 100) <= 10);
            
            if (forza_boss || chance_boss) {
                nodo_ss->nemico = demotorzone;
                flag_boss_generato = true;
            }
        }

        // Linking
        nodo_mr->link_soprasotto = nodo_ss;
        nodo_ss->link_mondoreale = nodo_mr;

        // Queuing
        inserisci_in_coda_reale(nodo_mr);
        inserisci_in_coda_soprasotto(nodo_ss);
    }

    flag_mappa_costruita = true;
    printf("Mappa generata e collegata con successo.\n");
    attendi_input_utente();
}

static void inserisci_in_coda_reale(struct Zona_mondoreale* nuovo_nodo) {
    if (radice_mappa_reale == NULL) {
        radice_mappa_reale = nuovo_nodo;
        return;
    }
    struct Zona_mondoreale* iteratore = radice_mappa_reale;
    while (iteratore->avanti != NULL) {
        iteratore = iteratore->avanti;
    }
    iteratore->avanti = nuovo_nodo;
    nuovo_nodo->indietro = iteratore;
}

static void inserisci_in_coda_soprasotto(struct Zona_soprasotto* nuovo_nodo) {
    if (radice_mappa_soprasotto == NULL) {
        radice_mappa_soprasotto = nuovo_nodo;
        return;
    }
    struct Zona_soprasotto* iteratore = radice_mappa_soprasotto;
    while (iteratore->avanti != NULL) {
        iteratore = iteratore->avanti;
    }
    iteratore->avanti = nuovo_nodo;
    nuovo_nodo->indietro = iteratore;
}

//Calcola la lunghezza della mappa per ora
static int calcola_lunghezza_mappa_attuale(void) {
    int contatore = 0;
    struct Zona_mondoreale* temp = radice_mappa_reale;
    while (temp != NULL) {
        contatore++;
        temp = temp->avanti;
    }
    return contatore;
}

//Aiuta nell'acquisizione di una nuova zona
static Tipo_zona acquisizione_manuale_tipo_zona(void) {
    printf("\nSeleziona TIPO DI ZONA:\n");
    printf("0. Bosco\n1. Scuola\n2. Laboratorio\n3. Caverna\n4. Strada\n");
    printf("5. Giardino\n6. Supermercato\n7. Centrale\n8. Deposito\n9. Polizia\n");
    printf("Scelta > ");
    return (Tipo_zona)acquisisci_intero_sicuro(0, 9);
}

static Tipo_nemico acquisizione_manuale_nemico_mr(void) {
    printf("\nSeleziona NEMICO (Mondo Reale):\n");
    printf("0. Nessuno\n1. Billi\n");
    printf("Scelta > ");
    return (Tipo_nemico)acquisisci_intero_sicuro(0, 1);
}

static Tipo_oggetto acquisizione_manuale_oggetto(void) {
    printf("\nSeleziona OGGETTO a terra:\n");
    printf("0. Nessuno\n1. Bicicletta\n2. Maglietta\n3. Bussola\n4. Chitarra\n");
    printf("Scelta > ");
    return (Tipo_oggetto)acquisisci_intero_sicuro(0, 4);
}

// Funzione Principale Inserimento
static void inserisci_zona_manuale(void) {
    stampa_intestazione_fase("INSERIMENTO MANUALE ZONA");
    
    int lunghezza_attuale = calcola_lunghezza_mappa_attuale();
    printf("Lunghezza mappa attuale: %d zone.\n", lunghezza_attuale);
    printf("Inserisci indice posizione (0 - %d): ", lunghezza_attuale);
    
    int posizione = acquisisci_intero_sicuro(0, lunghezza_attuale);
    
    // Acquisizione parametri
    Tipo_zona t_zona = acquisizione_manuale_tipo_zona();
    Tipo_nemico t_nemico_mr = acquisizione_manuale_nemico_mr();
    Tipo_oggetto t_oggetto = acquisizione_manuale_oggetto();
    
    // Allocazione
    struct Zona_mondoreale* nuovo_mr = alloca_nuovo_nodo_reale();
    struct Zona_soprasotto* nuovo_ss = alloca_nuovo_nodo_soprasotto();
    
    // Setup MR
    nuovo_mr->tipo = t_zona;
    nuovo_mr->nemico = t_nemico_mr;
    nuovo_mr->oggetto = t_oggetto;
    
    // Setup SS (Nemico randomico come da traccia, tipo speculare)
    nuovo_ss->tipo = t_zona;
    if (genera_intero_casuale(1, 100) <= SOGLIA_PROBABILITA_NEMICO) {
        nuovo_ss->nemico = democane;
    } else {
        nuovo_ss->nemico = nessun_nemico;
    }
    
    // Linking
    nuovo_mr->link_soprasotto = nuovo_ss;
    nuovo_ss->link_mondoreale = nuovo_mr;
    
    // Inserimento Lista
    collega_nodi_in_posizione_specifica(nuovo_mr, nuovo_ss, posizione);
    
    flag_mappa_costruita = true; 
    printf("\n>> Zona inserita correttamente in posizione %d.\n", posizione);
    attendi_input_utente();
}

static void collega_nodi_in_posizione_specifica(struct Zona_mondoreale* n_mr, struct Zona_soprasotto* n_ss, int pos) {
    // Caso TESTA (pos 0)
    if (pos == 0) {
        n_mr->avanti = radice_mappa_reale;
        if (radice_mappa_reale != NULL) radice_mappa_reale->indietro = n_mr;
        radice_mappa_reale = n_mr;
        
        n_ss->avanti = radice_mappa_soprasotto;
        if (radice_mappa_soprasotto != NULL) radice_mappa_soprasotto->indietro = n_ss;
        radice_mappa_soprasotto = n_ss;
        return;
    }
    
    // Caso CORPO/CODA
    struct Zona_mondoreale* curr_mr = radice_mappa_reale;
    struct Zona_soprasotto* curr_ss = radice_mappa_soprasotto;
    
    int i = 0;
    while (i < pos - 1 && curr_mr != NULL) {
        curr_mr = curr_mr->avanti;
        curr_ss = curr_ss->avanti;
        i++;
    }
    
    if (curr_mr == NULL || curr_ss == NULL) return; // Safety
    
    // MR Linking
    n_mr->avanti = curr_mr->avanti;
    n_mr->indietro = curr_mr;
    if (curr_mr->avanti != NULL) curr_mr->avanti->indietro = n_mr;
    curr_mr->avanti = n_mr;
    
    // SS Linking
    n_ss->avanti = curr_ss->avanti;
    n_ss->indietro = curr_ss;
    if (curr_ss->avanti != NULL) curr_ss->avanti->indietro = n_ss;
    curr_ss->avanti = n_ss;
}

// Funzione Principale Cancellazione
static void cancella_zona_manuale(void) {
    stampa_intestazione_fase("CANCELLAZIONE MANUALE ZONA");
    
    if (!flag_mappa_costruita || radice_mappa_reale == NULL) {
        printf("Errore: Mappa vuota o inesistente.\n");
        attendi_input_utente();
        return;
    }
    
    int len = calcola_lunghezza_mappa_attuale();
    printf("Lunghezza attuale: %d.\n", len);
    printf("Indice da cancellare (0 - %d): ", len - 1);
    
    int pos = acquisisci_intero_sicuro(0, len - 1);
    
    rimuovi_nodi_in_posizione_specifica(pos);
    
    printf("\n>> Zona %d rimossa.\n", pos);
    
    if (radice_mappa_reale == NULL) {
        flag_mappa_costruita = false;
        printf(">> ATTENZIONE: Mappa completamente vuota.\n");
    }
    attendi_input_utente();
}

static void rimuovi_nodi_in_posizione_specifica(int pos) {
    struct Zona_mondoreale* target_mr = radice_mappa_reale;
    struct Zona_soprasotto* target_ss = radice_mappa_soprasotto;
    
    // Ricerca
    int i = 0;
    while (i < pos && target_mr != NULL) {
        target_mr = target_mr->avanti;
        target_ss = target_ss->avanti;
        i++;
    }
    
    if (!target_mr || !target_ss) return;
    
    // Controllo Boss cancellato
    if (target_ss->nemico == demotorzone) {
        printf(">> ATTENZIONE: Hai cancellato la zona del DEMOTORZONE! Dovrai reinserirlo.\n");
        flag_boss_generato = false;
    }
    
    // Unlink MR
    if (target_mr == radice_mappa_reale) {
        radice_mappa_reale = target_mr->avanti;
        if (radice_mappa_reale) radice_mappa_reale->indietro = NULL;
    } else {
        target_mr->indietro->avanti = target_mr->avanti;
        if (target_mr->avanti) target_mr->avanti->indietro = target_mr->indietro;
    }
    
    // Unlink SS
    if (target_ss == radice_mappa_soprasotto) {
        radice_mappa_soprasotto = target_ss->avanti;
        if (radice_mappa_soprasotto) radice_mappa_soprasotto->indietro = NULL;
    } else {
        target_ss->indietro->avanti = target_ss->avanti;
        if (target_ss->avanti) target_ss->avanti->indietro = target_ss->indietro;
    }
    
    free(target_mr);
    free(target_ss);
}


static const char* converti_enum_zona_stringa(Tipo_zona z) {
    switch (z) {
        case bosco: return "Bosco Oscuro";
        case scuola: return "Scuola Elementare";
        case laboratorio: return "Laboratorio Segreto";
        case caverna: return "Caverna Umida";
        case strada: return "Strada Principale";
        case giardino: return "Giardino Pubblico";
        case supermercato: return "Supermercato";
        case centrale_elettrica: return "Centrale Elettrica";
        case deposito_abbandonato: return "Deposito Vecchio";
        case stazione_polizia: return "Stazione Polizia";
        default: return "Luogo Sconosciuto";
    }
}

static const char* converti_enum_nemico_stringa(Tipo_nemico n) {
    switch (n) {
        case nessun_nemico: return "Nessuno";
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "DEMOTORZONE (BOSS)";
        default: return "???";
    }
}

static const char* converti_enum_oggetto_stringa(Tipo_oggetto o) {
    switch (o) {
        case nessun_oggetto: return "Vuoto";
        case bicicletta: return "Bicicletta";
        case maglietta_fuocoinferno: return "Maglietta Fuocoinferno";
        case bussola: return "Bussola";
        case schitarrata_metallica: return "Chitarra Metallica";
        default: return "Oggetto Misterioso";
    }
}


static void configura_singolo_giocatore(int indice) {
    elenco_giocatori_globali[indice] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
    struct Giocatore* g = elenco_giocatori_globali[indice];
    
    if (g == NULL) exit(EXIT_FAILURE);

    printf("\n--- Configurazione Giocatore %d ---\n", indice + 1);
    printf("Inserisci il nome: ");
    scanf("%63s", g->nome);
    pulisci_flusso_input();

    g->mondo = 0;
    g->vivo = true;
    g->oggetti_posseduti = 0;
    for(int k=0; k<DIMENSIONE_ZAINO_GIOCATORE; k++) g->zaino[k] = nessun_oggetto;

    g->attacco_pischico = genera_intero_casuale(1, 20);
    g->difesa_pischica = genera_intero_casuale(1, 20);
    g->fortuna = genera_intero_casuale(1, 20);

    printf("Statistiche base     ATT: %d | DIF: %d | FORT: %d\n", 
           g->attacco_pischico, g->difesa_pischica, g->fortuna);

    printf("Scegli un modificatore iniziale:\n");
    printf("1. Aggressivo (+3 ATT, -3 DIF)\n");
    printf("2. Difensivo (-3 ATT, +3 DIF)\n");
    printf("3. Speciale: Undici VirgolaCinque (+4 ATT, +4 DIF, -7 FORT)\n");
    printf("4. Nessuna modifica\n");
    printf("Scelta ");
    
    int scelta = acquisisci_intero_sicuro(1, 4);
    
    switch(scelta) {
        case 1:
            g->attacco_pischico += 3;
            g->difesa_pischica -= 3;
            break;
        case 2:
            g->attacco_pischico -= 3;
            g->difesa_pischica += 3;
            break;
        case 3:
            g->attacco_pischico += 4;
            g->difesa_pischica += 4;
            g->fortuna -= 7;
            strcat(g->nome, " (11.5)"); 
            break;
        default: break;
    }

    if (g->attacco_pischico < 1) g->attacco_pischico = 1;
    if (g->difesa_pischica < 1) g->difesa_pischica = 1;
    if (g->fortuna < 1) g->fortuna = 1;
}

static void esegui_turno_gioco(struct Giocatore* g_corr) {
    if (!g_corr->vivo) return;

    bool turno_attivo = true;
    bool movimento_effettuato = false;

    stampa_linea_separatrice();
    printf("TURNO DI GIOCO: %s\n", g_corr->nome);
    stampa_descrizione_luogo(g_corr);
    stampa_linea_separatrice();

    while (turno_attivo && g_corr->vivo) {
        printf("\nOpzioni disponibili:\n");
        printf("1. Avanza (Muoversi in avanti)\n");
        printf("2. Indietreggia (Muoversi indietro)\n");
        printf("3. Cambia Mondo (Passaggio dimensionale)\n");
        printf("4. Combatti (Attacca nemico presente)\n");
        printf("5. Raccogli Oggetto (Solo Mondo Reale)\n");
        printf("6. Usa Oggetto (Dallo zaino)\n");
        printf("7. Passa il turno\n");
        printf("Seleziona azione ");

        int azione = acquisisci_intero_sicuro(1, 7);

        switch (azione) {
            case 1:
                if (movimento_effettuato) {
                    printf("Hai gia' effettuato un movimento in questo turno.\n");
                } else {
                    gestisci_azione_avanzamento(g_corr);
                    movimento_effettuato = true;
                }
                break;
            case 2: 
                if (movimento_effettuato) {
                    printf("Hai gia' effettuato un movimento in questo turno.\n");
                } else {
                    gestisci_azione_arretramento(g_corr);
                    movimento_effettuato = true;
                }
                break;
            case 3:
                if (movimento_effettuato) {
                    printf("Non puoi cambiare dimensione dopo esserti mosso.\n");
                } else {
                    gestisci_azione_cambio_dimensione(g_corr);
                    movimento_effettuato = true; 
                }
                break;
            case 4:
                gestisci_scontro_nemico(g_corr);
                if (!g_corr->vivo) turno_attivo = false;
                break;
            case 5:
                gestisci_raccolta_risorse(g_corr);
                break;
            case 6:
                gestisci_utilizzo_inventario(g_corr);
                break;
            case 7:
                printf("%s termina il turno.\n", g_corr->nome);
                turno_attivo = false;
                break;
        }
    }
}

static void stampa_descrizione_luogo(struct Giocatore* g_ptr) {
    Tipo_zona zona_tipo;
    Tipo_nemico nemico_tipo;
    const char* dim_str;

    if (g_ptr->mondo == 0) {
        zona_tipo = g_ptr->pos_mondoreale->tipo;
        nemico_tipo = g_ptr->pos_mondoreale->nemico;
        dim_str = "MONDO REALE";
    } else {
        zona_tipo = g_ptr->pos_soprasotto->tipo;
        nemico_tipo = g_ptr->pos_soprasotto->nemico;
        dim_str = "SOPRASOTTO";
    }

    printf("Dimensione: %s\n", dim_str);
    printf("Luogo: %s\n", converti_enum_zona_stringa(zona_tipo));
    printf("Minaccia: %s\n", converti_enum_nemico_stringa(nemico_tipo));
}


static void gestisci_scontro_nemico(struct Giocatore* g) {
    Tipo_nemico nemico_presente;
    Tipo_nemico* puntatore_modifica_nemico;

    if (g->mondo == 0) {
        nemico_presente = g->pos_mondoreale->nemico;
        puntatore_modifica_nemico = &(g->pos_mondoreale->nemico);
    } else {
        nemico_presente = g->pos_soprasotto->nemico;
        puntatore_modifica_nemico = &(g->pos_soprasotto->nemico);
    }

    if (nemico_presente == nessun_nemico) {
        printf("Non c'e' nessuno da combattere qui.\n");
        return;
    }

    int forza_nemico = 0;
    if (nemico_presente == billi) forza_nemico = 10;
    else if (nemico_presente == democane) forza_nemico = 15;
    else if (nemico_presente == demotorzone) forza_nemico = 25;

    printf("\nCOMBATTIMENTO INIZIATO\n");
    bool combattimento_attivo = true;

    while (combattimento_attivo && g->vivo) {
        printf("Premi INVIO per attaccare...");
        pulisci_flusso_input();

        int tiro_giocatore = genera_intero_casuale(1, 20);
        int totale_giocatore = tiro_giocatore + g->attacco_pischico;

        int tiro_nemico = genera_intero_casuale(1, 20);
        int totale_nemico = tiro_nemico + forza_nemico;

        printf("Tuo Colpo: %d (Dado) + %d (Stat) = %d\n", tiro_giocatore, g->attacco_pischico, totale_giocatore);
        printf("Nemico: %d (Dado) + %d (Stat) = %d\n", tiro_nemico, forza_nemico, totale_nemico);

        if (totale_giocatore >= totale_nemico) {
            printf("HAI VINTO LO SCONTRO!\n");
            combattimento_attivo = false;

            if (nemico_presente == demotorzone) {
                printf("\nDEMOTORZONE SCONFITTO! VITTORIA!\n");
                aggiorna_registro_vincitori(g->nome);
                termina_gioco();
                exit(EXIT_SUCCESS);
            }

            if (genera_intero_casuale(0, 1) == 1) {
                *puntatore_modifica_nemico = nessun_nemico;
                printf("Il nemico fugge via.\n");
            } else {
                *puntatore_modifica_nemico = nessun_nemico; 
                printf("Il nemico e' a terra.\n");
            }
        } else {
            printf("SEI STATO COLPITO!\n");
            int danno = totale_nemico - g->difesa_pischica;
            
            if (danno > 0) {
                int salvezza = genera_intero_casuale(1, 20);
                printf("Tiro Salvezza (Fortuna): %d (Necessario > %d)\n", salvezza, g->fortuna);
                
                if (salvezza > g->fortuna) {
                    printf("Colpo fatale! Sei morto.\n");
                    g->vivo = false;
                    combattimento_attivo = false;
                } else {
                    printf("Sei ferito ma sopravvivi per miracolo.\n");
                }
            } else {
                printf("La tua difesa ha assorbito il colpo.\n");
            }
        }
    }
}

static void gestisci_azione_avanzamento(struct Giocatore* g) {
    Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;
    if (n != nessun_nemico) {
        printf("Impossibile avanzare: nemico presente! Devi combattere.\n");
        gestisci_scontro_nemico(g);
        if (!g->vivo) return; 
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->avanti != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            printf("Sei avanzato nel Mondo Reale.\n");
        } else {
            printf("Sei alla fine della mappa.\n");
        }
    } else {
        if (g->pos_soprasotto->avanti != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            printf("Sei avanzato nel Soprasotto.\n");
        } else {
            printf("Sei alla fine della mappa oscura.\n");
        }
    }
}

static void gestisci_azione_arretramento(struct Giocatore* g) {
    Tipo_nemico n = (g->mondo == 0) ? g->pos_mondoreale->nemico : g->pos_soprasotto->nemico;
    if (n != nessun_nemico) {
        printf("Non puoi voltare le spalle al nemico!\n");
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            printf("Sei indietreggiato nel Mondo Reale.\n");
        } else {
            printf("Sei all'inizio della mappa.\n");
        }
    } else {
        if (g->pos_soprasotto->indietro != NULL) {
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            printf("Sei indietreggiato nel Soprasotto.\n");
        } else {
            printf("Sei all'inizio della mappa oscura.\n");
        }
    }
}

static void gestisci_azione_cambio_dimensione(struct Giocatore* g) {
    if (g->mondo == 0) {
        if (g->pos_mondoreale->nemico != nessun_nemico) {
            printf("Non puoi aprire il portale con nemici vicini.\n");
            return;
        }
        g->mondo = 1;
        g->pos_soprasotto = g->pos_mondoreale->link_soprasotto;
        printf("Sei stato trasportato nel SOPRASOTTO.\n");
    } else {
        int dado_fuga = genera_intero_casuale(1, 20);
        printf("Tiro Fuga: %d (La tua Fortuna e' %d)\n", dado_fuga, g->fortuna);
        
        if (dado_fuga < g->fortuna) {
            g->mondo = 0;
            g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
            printf("Successo! Sei tornato nel Mondo Reale.\n");
        } else {
            printf("Fallimento! Il portale si e' chiuso davanti a te.\n");
        }
    }
}

static void gestisci_raccolta_risorse(struct Giocatore* g) {
    if (g->mondo == 1) {
        printf("Non ci sono oggetti utili nel Soprasotto.\n");
        return;
    }
    if (g->pos_mondoreale->nemico != nessun_nemico) {
        printf("Sconfiggi prima il nemico!\n");
        return;
    }
    if (g->pos_mondoreale->oggetto == nessun_oggetto) {
        printf("Nessun oggetto in questa zona.\n");
        return;
    }
    if (g->oggetti_posseduti >= DIMENSIONE_ZAINO_GIOCATORE) {
        printf("Zaino pieno! Usa qualcosa prima.\n");
        return;
    }

    g->zaino[g->oggetti_posseduti] = g->pos_mondoreale->oggetto;
    g->oggetti_posseduti++;
    g->pos_mondoreale->oggetto = nessun_oggetto; 
    printf("Oggetto raccolto e messo nello zaino.\n");
}

static void gestisci_utilizzo_inventario(struct Giocatore* g) {
    if (g->oggetti_posseduti == 0) {
        printf("Zaino vuoto.\n");
        return;
    }

    printf("Contenuto Zaino:\n");
    for (int i = 0; i < g->oggetti_posseduti; i++) {
        printf("%d) %s\n", i + 1, converti_enum_oggetto_stringa(g->zaino[i]));
    }
    printf("Quale oggetto usare? (1-%d): ", g->oggetti_posseduti);
    
    int indice = acquisisci_intero_sicuro(1, g->oggetti_posseduti) - 1;
    Tipo_oggetto usato = g->zaino[indice];

    switch (usato) {
        case bicicletta:
            g->fortuna += 2;
            printf("Hai usato Bicicletta: Fortuna +2.\n");
            break;
        case maglietta_fuocoinferno:
            g->attacco_pischico += 2;
            printf("Hai usato Maglietta: Attacco +2.\n");
            break;
        case schitarrata_metallica:
            g->difesa_pischica += 3;
            printf("Hai usato Chitarra: Difesa +3.\n");
            break;
        default: break;
    }

    for (int j = indice; j < g->oggetti_posseduti - 1; j++) {
        g->zaino[j] = g->zaino[j + 1];
    }
    g->oggetti_posseduti--;
}

static void aggiorna_registro_vincitori(const char* nome_vincitore) {
    strcpy(registro_vincitori[2], registro_vincitori[1]);
    strcpy(registro_vincitori[1], registro_vincitori[0]);
    strcpy(registro_vincitori[0], nome_vincitore);

    if (conteggio_vincitori_registrati < NUMERO_VINCITORI_MEMORIZZATI) {
        conteggio_vincitori_registrati++;
    }
}


void imposta_gioco() {
    resetta_memoria_completa();
    stampa_intestazione_fase("CONFIGURAZIONE NUOVA PARTITA");

    printf("Inserire numero giocatori (1-4): ");
    numero_giocatori_attuali = acquisisci_intero_sicuro(1, 4);

    elenco_giocatori_globali = (struct Giocatore**) malloc(numero_giocatori_attuali * sizeof(struct Giocatore*));
    if (elenco_giocatori_globali == NULL) exit(EXIT_FAILURE);

    for (int i = 0; i < numero_giocatori_attuali; i++) {
        configura_singolo_giocatore(i);
    }

    int scelta_mappa = 0;
    while (scelta_mappa != 5) {
        printf("MENU GESTIONE MAPPA\n");
        printf("1. Genera Mappa Casuale \n");
        printf("2. Inserisci Nuova Zona \n");
        printf("3. Cancella Zona Esistente\n");
        printf("4. Visualizza Mappa \n");
        printf("5. Termina Configurazione e Gioca\n");
        printf("Comando");
        
        scelta_mappa = acquisisci_intero_sicuro(1, 5);

        switch (scelta_mappa) {
            case 1:
                procedura_creazione_mappa();
                break;
            case 2:
                inserisci_zona_manuale();
                break;
            case 3:
                cancella_zona_manuale();
                break;
            case 4:
                if (!flag_mappa_costruita) {
                    printf(">> ERRORE: Nessuna mappa in memoria.\n");
                } else {
                    printf("\n[MAPPA ATTUALE - %d Zone]\n", calcola_lunghezza_mappa_attuale());
                    struct Zona_mondoreale* tmp = radice_mappa_reale;
                    int idx = 0;
                    while(tmp) {
                        printf("[%d] %s (Nemico: %s)\n", idx++, converti_enum_zona_stringa(tmp->tipo), converti_enum_nemico_stringa(tmp->nemico));
                        tmp = tmp->avanti;
                    }
                }
                attendi_input_utente();
                break;
            case 5:
                if (!flag_mappa_costruita) {
                    printf("ERRORE BLOCCANTE: Devi generare almeno una zona.\n");
                    scelta_mappa = 0; 
                } 
                else if (calcola_lunghezza_mappa_attuale() < NUMERO_TOTALE_ZONE_MAPPA) {
                    printf("ERRORE BLOCCANTE: La mappa deve avere almeno %d zone (Attuali: %d).\n", 
                           NUMERO_TOTALE_ZONE_MAPPA, calcola_lunghezza_mappa_attuale());
                    scelta_mappa = 0;
                }
                else if (!flag_boss_generato) {
                    // Check boss manuale
                    bool boss_trovato = false;
                    struct Zona_soprasotto* scan = radice_mappa_soprasotto;
                    while(scan) { if(scan->nemico == demotorzone) boss_trovato = true; scan = scan->avanti; }
                    
                    if (!boss_trovato) {
                        printf(">> ERRORE BLOCCANTE: Manca il DEMOTORZONE!\n");
                        scelta_mappa = 0;
                    }
                }
                
                if (scelta_mappa == 5) {
                    printf(">> Validazione Mappa: OK.\n");
                    for(int k=0; k<numero_giocatori_attuali; k++) {
                        elenco_giocatori_globali[k]->pos_mondoreale = radice_mappa_reale;
                        elenco_giocatori_globali[k]->pos_soprasotto = radice_mappa_soprasotto;
                    }
                    flag_gioco_inizializzato = true;
                }
                break;
        }
    }
    printf("Configurazione completata. Pronto a giocare.\n");
}

void gioca() {
    if (!flag_gioco_inizializzato) {
        printf("Errore: Devi prima impostare il gioco!\n");
        return;
    }

    stampa_intestazione_fase("INIZIO PARTITA");
    bool partita_in_corso = true;

    while (partita_in_corso) {
        int contatore_vivi = 0;
        for (int i = 0; i < numero_giocatori_attuali; i++) {
            if (elenco_giocatori_globali[i]->vivo) contatore_vivi++;
        }

        if (contatore_vivi == 0) {
            printf("Tutti i giocatori sono morti. GAME OVER.\n");
            termina_gioco();
            return;
        }

        for (int i = 0; i < numero_giocatori_attuali; i++) {
            esegui_turno_gioco(elenco_giocatori_globali[i]);
        }
    }
}

void termina_gioco() {
    stampa_intestazione_fase("TERMINAZIONE GIOCO");
    resetta_memoria_completa();
    printf("Sessione conclusa. Arrivederci.\n");
}

void crediti() {
    stampa_intestazione_fase("CREDITI E ULTIMI VINCITORI");
    printf("Sviluppato da: Maciej Lakomski\n");
    printf("Progetto Universitario: Cosestrane\n");
    printf("\nVINCITORI (ULTIME 3 PARTITE) \n");
    
    if (conteggio_vincitori_registrati == 0) {
        printf("(Nessun vincitore registrato finora)\n");
    } else {
        for (int i = 0; i < conteggio_vincitori_registrati; i++) {
            printf("%d. %s\n", i + 1, registro_vincitori[i]);
        }
    }
    attendi_input_utente();
}