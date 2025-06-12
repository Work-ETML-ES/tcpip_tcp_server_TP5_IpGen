// Canevas manipulation GenSig avec menu
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber


#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "system_config.h"
#include "Mc32NVMUtil.h"
#include "Mc32DriverLcd.h"
#include <math.h>
#include "Mc32NVMUtil.h"

// Variables globales
S_ParamGen valeursParamGen;
int32_t tableauValeursSignal[MAX_ECH];

//----------------------------------------------------------------------------
//  GENSIG_Initialize
//  Initialise le générateur à partir des données en NVM ou valeurs par défaut
//----------------------------------------------------------------------------

void GENSIG_Initialize(S_ParamGen *pParam) {
    // Lecture du bloc mémoire sauvé précédemment
    NVM_ReadBlock((uint32_t*) & valeursParamGen, sizeof (S_ParamGen));

    // Vérification de l'authenticité des données sauvegardées
    if (valeursParamGen.Magic != MAGIC) {
        // Cas invalide : utilisation des valeurs par défaut
        pParam->Amplitude = 10000;
        pParam->Forme = SignalSinus;
        pParam->Frequence = 20;
        pParam->Magic = MAGIC;
        pParam->Offset = 0;
    } else {
        // Cas valide : restauration des données
        *pParam = valeursParamGen;
    }
}

//----------------------------------------------------------------------------
//  GENSIG_UpdatePeriode
//  Met à jour la période d?échantillonnage en fonction de la fréquence
//----------------------------------------------------------------------------

void GENSIG_UpdatePeriode(S_ParamGen *pParam) {
    static uint16_t compteurTimer3;
    // Ajustement du timer en fonction de la fréquence
    compteurTimer3 = (uint16_t) ((TRANSFORMATION_VALEUR_TIMER3 / pParam->Frequence));
    PLIB_TMR_Period16BitSet(TMR_ID_3, compteurTimer3);
}

//-------------------------------
// Mise à jour du signal (forme, amplitude, offset)
// Entrées : Pointeur sur la structure S_ParamGen : pParam
// Sortie  : -
//-------------------------------

void GENSIG_UpdateSignal(S_ParamGen *pParam) {
    uint8_t nbEchantillon = 0;
    uint16_t amplitude = 0;
    int16_t offset = 0;

    // Calcul de l'échelle amplitude et offset
    amplitude = pParam->Amplitude / 100;
    offset = pParam->Offset / 2;

    // Parcours de tous les échantillons
    for (nbEchantillon = 0; nbEchantillon < MAX_ECH; nbEchantillon++) {
        int32_t valeurBrute = 0; // valeur brute avant écrêtage et conversion

        switch (pParam->Forme) {
            case SignalSinus:
            {
                // Calcul de l'angle en radians pour nbEchantillon
                float angle = 2 * (float) M_PI * ((float) nbEchantillon / MAX_ECH);

                // On utilise sin(angle) qui varie entre -1 et +1
                float sinusFloat = sinf(angle) * MOITIE_ECH;

                // application de l'amplitude
                int32_t sinusScaled = (int32_t) (sinusFloat) * amplitude;

                valeurBrute = (int32_t) (MOITIE_AMPLITUDE - offset) + sinusScaled;
            }
                break;

            case SignalTriangle:
            {
                // Montée sur la première moitié, descente sur la seconde
                if (nbEchantillon < (MAX_ECH / 2)) {
                    valeurBrute = (int32_t) (MOITIE_AMPLITUDE - offset)
                            + (int32_t) (amplitude * (2 * (nbEchantillon - 25)));
                } else {
                    valeurBrute = (int32_t) (MOITIE_AMPLITUDE - offset)
                            + (int32_t) (amplitude * (100 - 2 * (nbEchantillon - 25)));
                }
            }
                break;

            case SignalDentDeScie:
            {
                // Valeur linéaire sur MAX_ECH points
                valeurBrute = (int32_t) (MOITIE_AMPLITUDE - offset)
                        + (int32_t) ((nbEchantillon - 50) * amplitude);
            }
                break;

            case SignalCarre:
            {
                // Hauteur max sur la première moitié, hauteur min sur la seconde
                if (nbEchantillon < (MAX_ECH / 2)) {
                    valeurBrute = (int32_t) (MOITIE_AMPLITUDE)
                            + (int32_t) (amplitude / 2 * (float) MAX_ECH)
                            - (int32_t) offset;
                } else {
                    valeurBrute = (int32_t) (MOITIE_AMPLITUDE)
                            - (int32_t) ((amplitude / 2 * (float) MAX_ECH) + offset);
                }
            }
                break;
            default:
                break;
        }

        // Écrêtage : borne la valeur entre 0 et MAX_AMPLITUDE
        if (valeurBrute > MAX_AMPLITUDE) {
            valeurBrute = MAX_AMPLITUDE;
        } else if (valeurBrute < 0) {
            valeurBrute = 0;
        }

        // Mise à l'échelle finale 0..10000 => 0..VAL_MAX_PAS
        tableauValeursSignal[nbEchantillon] =
                (int64_t) ((VAL_MAX_PAS * valeurBrute) / 10000);
    }
}

//----------------------------------------------------------------------------
//  GENSIG_Execute
//  Envoie cycliquement chaque échantillon au DAC
//----------------------------------------------------------------------------

void GENSIG_Execute(void) {
    static uint16_t EchNb = 0;

    // Écriture sur le DAC du prochain échantillon
    SPI_WriteToDac(0, tableauValeursSignal[EchNb]);

    // Passage à l'échantillon suivant et gestion du débordement
    EchNb = (uint16_t) ((EchNb + 1) % MAX_ECH);
}
