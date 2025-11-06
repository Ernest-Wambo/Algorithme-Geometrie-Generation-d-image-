//
// Created by wambo on 11/6/2025.
//

#include <iostream>
#include <string>
#include "OutilsCreationImage.h"
#include <math.h>

using namespace std;

/*========================================================================
DM Noté : Du nombre au pixel
Creation de Saturne et de ses anneaux

Année 2023/2024

Jean Ernest Wambo (univ lorraine, metz France)
==========================================================================*/

const unsigned long MASQUE = 255;
const double IMAX = 255;
const unsigned long LARGEUR=760;
const unsigned long HAUTEUR=428;

const unsigned long couleurBleuNuit=0x19197000; //bleu nuit complétement transparent
const unsigned long couleurJaunePale=0xFFFF99FF; //jaune pâle opaque
const unsigned long couleurRouge = 0xFF0000FF;
const unsigned long couleurNoir = 0x000000FF;

// fonction distance
double distance(double x1, double y1, double x2, double y2){
	double deltaX=x1-x2;
	double deltaY=y1-y2;

	return deltaX*deltaX+deltaY*deltaY;
}

//Obtenir les anneaux de saturne

//=============================================================================
//fonction 1
bool appartientEllipse(double i, double j, double i0, double j0, double a, double b){

	double deltI = i - i0;
	double deltJ = j - j0;
	double coefa = (deltI * deltI) / (a * a);
	double coefb = (deltJ * deltJ) / (b * b);

	return ((coefa + coefb) <= 1.0);
}

//fonction 2
bool appartientAnneau(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){

	bool ellipseExt = appartientEllipse(i, j, i0, j0, a1, b1);
	bool ellipseInt = appartientEllipse(i, j, i0, j0, a2, b2);

	return ellipseExt && !ellipseInt;
}

//fonction 3
bool appartientAnneauGauche(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){
	return (appartientAnneau(i, j, i0, j0, a1, b1, a2, b2) && i <= i0);
}

//fonction 4
bool appartientAnneauDroite(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){
	return (appartientAnneau(i, j, i0, j0, a1, b1, a2, b2) && i > i0);
}
//=============================================================================

bool appartientAnneauSaturne(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){
	double c = (a1+a2)/2;
	double d = (b1+b2)/2;

	return (appartientAnneau(i, j, i0, j0, a1, b1, a2, b2) && !appartientAnneau(i, j, i0, j0, c+3, d+3, c-3, d-3));
}


//===========================================================================
//fonction F(x,y)
bool appartientEllipseOmbre(double i, double j, double i0, double j0, double a, double b, double p, double q){
	double deltI = i - i0; //y
	double deltJ = j -j0; //x

	double u = (p * deltJ) + (q * deltI);
	double v = (-q * deltJ) + (p * deltI);

	double coefa = (u * u) / (a * a);
	double coefb = (v * v) / (b * b);

	return (( coefa + coefb ) <= 1.0);
}

//===========================================================================

bool appartientDisque(double rayon, double i0, double j0, double i, double j){
	double distanceCaree = distance(i0, j0, i, j);

	return (distanceCaree<=rayon*rayon);
}


//obtenir a pour degrader

//==========================================================================
//fonction f(x,y)
double distanceEllipse(double i, double j, double i0, double j0, double a, double b){
	double deltI = i - i0;
	double deltJ = j - j0;
	double coefa = (deltI * deltI) / (a * a);
	double coefb = (deltJ * deltJ) / (b * b);

	return (fabs((coefa + coefb - 1)));
}

double distanceAnneau(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){
	double distanceExt = distanceEllipse(i, j, i0, j0, a1, b1);
	double distanceInt = distanceEllipse(i, j, i0, j0, a2, b2);

	double distanceNorm = distanceExt / (distanceExt + distanceInt);

	return (fabs(distanceNorm - 0.5)*0.75);
}
//==========================================================================

double distanceAnneauSaturne(double i, double j, double i0, double j0, double a1, double b1, double a2, double b2){
	double a = distanceAnneau(i, j, i0, j0, a1, b1, a2, b2);

	double c = (a1+a2)/2;
	double d = (b1+b2)/2;

	if (appartientEllipse(i, j, i0, j0, c-3, d-3))
		a = distanceAnneau(i, j, i0, j0, c-3, d-3 ,a2, b2);
	else
		a = distanceAnneau(i, j, i0, j0, a1, b1, c+3, d+3);

	return a;
}


//dégradé des anneaux
unsigned long extraitOctet (unsigned long x, int shift){
	return (x >> shift) & MASQUE;
}

double extraitComposanteReelle (unsigned long couleur, int shift){
	return extraitOctet(couleur, shift)/IMAX;
}

void remplitOctet (unsigned long &x, int shift, unsigned long v){
	x = x | (v << shift);
}

void remplitComposanteReelle (unsigned long &x, int shift, double v){
	remplitOctet(x,shift,(unsigned long)(v*IMAX));
}

unsigned long monBlend3couleur (double a, unsigned long couleurRGBA1, unsigned long couleurRGBA2, unsigned long couleurRGBA3){
	unsigned long f = 0;
	int i, shift;

	for (i=0, shift = 0; i<= 3; ++i, shift =shift+8){
		double i1 = extraitComposanteReelle(couleurRGBA1,shift);
		double i2 = extraitComposanteReelle(couleurRGBA2,shift);
		double i3 = extraitComposanteReelle(couleurRGBA3,shift);

		double iR = 0;

		if (a < 0.5)
			iR = (1 - 2*a) * i1 + (1 + 2*a) * i2;
		else
			iR = (2 - 2*a) * i2 + (2*a - 1) * i3;

		remplitComposanteReelle(f,shift,iR);
	}
	return f;
}

unsigned long monBlend2couleur (double a, unsigned long couleurRGBA1, unsigned long couleurRGBA2){
	double abar = 1-a;
	unsigned long f = 0;
	int i, shift;
	for (i=0, shift = 0; i<= 3; ++i, shift =shift+8){
		double i1 = extraitComposanteReelle(couleurRGBA1,shift);
		double i2 = extraitComposanteReelle(couleurRGBA2,shift);

		double iR = abar * i1 + a * i2;

		remplitComposanteReelle(f,shift,iR);
	}
	return f;
}


//creation de l'image
int main(){
	cout<<"creation d'une image representant de saturne"<<endl;
	string chemin="images_crees";

	string nomFichierImage=chemin+"\\"+"saturne.bmp";


	// hauteur de l'image en pixels

	unsigned long m=HAUTEUR-1;
	unsigned long n=LARGEUR-1;
	double a = 0;

	unsigned long matricePixels[HAUTEUR][LARGEUR];
	unsigned long i,j,k;

	unsigned long i0= m/2;
	unsigned long j0= n/2;

	//Algorithme du peintre

	//Anneaux supérieur
	for(i=0;i<=m;++i){
		for(j=0;j<=n;++j)
		{
			if ( appartientAnneauSaturne(i, j, i0, j0, 100, 360, 50, 200) && (i <= i0)){

				a = distanceAnneauSaturne(i, j, i0, j0, 100, 360, 50, 200);
				double couleur = monBlend3couleur(a, couleurJaunePale, couleurRouge, couleurJaunePale);

				if (appartientEllipseOmbre(i, j, 150, 500, 100, 200, 1, 1))
					k = monBlend2couleur(0.5, couleur, couleurNoir);
				else
					k = couleur;
			}
			else
				k= couleurBleuNuit;
			matricePixels[i][j]= k;
		}
	}

	//Saturne
	double blendCentreJ = 340;
	double blendCentreI = 165;
	double rayon = 115;

	for(i=0;i<=m;++i){
		for(j=0;j<=n;++j)
		{
			if ( appartientDisque(rayon, i0, j0, i, j) ){

				a = sqrt(distance(blendCentreI, blendCentreJ,i, j))/((i0+j0)-(blendCentreI+blendCentreJ)+rayon);
				k = monBlend2couleur(a, couleurJaunePale, couleurRouge);
				matricePixels[i][j]= k;
			}
		}
	}

	//Anneaux inférieur
	for(i=0;i<=m;++i){
		for(j=0;j<=n;++j)
		{
			if ( appartientAnneauSaturne(i, j, i0, j0, 100, 360, 50, 200) && (i > i0)){
				a = distanceAnneauSaturne(i, j, i0, j0, 100, 360, 50, 200);
				k = monBlend3couleur(a, couleurJaunePale, couleurRouge, couleurJaunePale);
				matricePixels[i][j]= k;
			}
		}
	}

		OutilsCreationImage :: creeImage(nomFichierImage, matricePixels, HAUTEUR);
		cout<<"image creee"<<endl;
		return 0;
}