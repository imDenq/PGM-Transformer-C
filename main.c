#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//------------------------------------------------------------------------------------------------------------------
//
// - Gestion de l'image PGM
//
//------------------------------------------------------------------------------------------------------------------

struct GrayImage
{
    int width;
    int height;
    unsigned char *pixels; // Tableau de données des pixels (niveaux de gris de 0 à 255)
};

struct GrayImage readPGM(const char *filename)
{
    struct GrayImage image;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }

    // Lire l'en-tête PGM (par exemple, "P5\n800 600\n255\n")
    char format[3];
    fscanf(file, "%2s", format);
    if (format[0] != 'P' || format[1] != '5')
    {
        fprintf(stderr, "Format de fichier PGM incorrect\n");
        fclose(file);
        exit(1);
    }

    fscanf(file, "%d %d", &image.width, &image.height);
    int maxVal;
    fscanf(file, "%d", &maxVal);
    if (maxVal != 255)
    {
        fprintf(stderr, "La valeur maximale doit être 255\n");
        fclose(file);
        exit(1);
    }

    image.pixels = (unsigned char *)malloc(image.width * image.height * sizeof(unsigned char));
    if (image.pixels == NULL)
    {
        perror("Erreur d'allocation de mémoire");
        fclose(file);
        exit(1);
    }

    fread(image.pixels, sizeof(unsigned char), image.width * image.height, file);

    fclose(file);
    return image;
}

void writePGM(const struct GrayImage *image, const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }
    fprintf(file, "P5\n%d %d\n255\n", image->width, image->height);

    if (image->pixels != NULL)
    {
        fwrite(image->pixels, sizeof(unsigned char), image->width * image->height, file);
    }

    fclose(file);
}

//------------------------------------------------------------------------------------------------------------------
//
// - Fonctions / effets sur image pgm
//
//------------------------------------------------------------------------------------------------------------------

void rotateImage(struct GrayImage *image, int angle)
{
    int newWidth = image->width; // variable pour la largeur
    int newHeight = image->height; // variable pour la hauteur
    unsigned char *newPixels = (unsigned char *)malloc(newWidth * newHeight * sizeof(unsigned char)); //alloc

    double radians = angle * 3.14159265358979323846 / 180.0; // conv l'angle en rd tgn
    int centerX = newWidth / 2; //calc X
    int centerY = newHeight / 2; //calc Y

    for (int y = 0; y < newHeight; y++) //parc Y
    {
        for (int x = 0; x < newWidth; x++) //parc X
        {
            //calc new X and Y a partir de l'angle et les coordonnées du centre de l'image
            int newX = cos(radians) * (x - centerX) - sin(radians) * (y - centerY) + centerX; 
            int newY = sin(radians) * (x - centerX) + cos(radians) * (y - centerY) + centerY;

            if (newX >= 0 && newX < newWidth && newY >= 0 && newY < newHeight) //ver si new cords = dim de l'image
            {
                newPixels[y * newWidth + x] = image->pixels[newY * newWidth + newX]; //ver si new cords in new image
            }
            else
            {
                newPixels[y * newWidth + x] = 0; //sinon mettre 0
            }
        }
    }

    free(image->pixels);
    image->pixels = newPixels;
}

void mirrorHorizontal(struct GrayImage *image)
{
    for (int i = 0; i < image->height; i++)
    {
        for (int j = 0; j < image->width / 2; j++)
        {
            int temp = image->pixels[i * image->width + j];
            image->pixels[i * image->width + j] = image->pixels[i * image->width + (image->width - 1 - j)];
            image->pixels[i * image->width + (image->width - 1 - j)] = temp;
        }
    }
}

void adjustContrast(struct GrayImage *image, double factor)
{
    for (int i = 0; i < image->width * image->height; i++)
    {

        // 128 > 0 - 128 < 255 a update
        int newValue = (int)(image->pixels[i] * factor);
        if (newValue < 0)
            newValue = 0;
        if (newValue > 255)
            newValue = 255;
        image->pixels[i] = (unsigned char)newValue;
    }
}

void adjustBrightness(struct GrayImage *image, int value)
{
    for (int i = 0; i < image->width * image->height; i++)
    {
        int newValue = image->pixels[i] + value;
        if (newValue < 0)
            newValue = 0;
        if (newValue > 255)
            newValue = 255;
        image->pixels[i] = (unsigned char)newValue;
    }
}

void applyThresholding(struct GrayImage *image, int threshold)
{
    for (int i = 0; i < image->width * image->height; i++)
    {
        if (image->pixels[i] < threshold)
            image->pixels[i] = 0;
        else
            image->pixels[i] = 255;
    }
}

void translateImage(struct GrayImage *image, int x, int y)
{
    // Allouer un nouveau tableau de pixels pour l'image traduite
    unsigned char *newPixels = (unsigned char *)malloc(image->width * image->height * sizeof(unsigned char));
    if (newPixels == NULL)
    {
        perror("Erreur d'allocation mémoire pour newPixels");
        exit(1);
    }

    // Normaliser les valeurs de translation pour qu'elles soient dans les limites de l'image
    x = x % image->width;
    y = y % image->height;
    if (x < 0)
        x += image->width; // Assurer une translation positive pour x
    if (y < 0)
        y += image->height; // Assurer une translation positive pour y

    for (int i = 0; i < image->height; i++)
    {
        for (int j = 0; j < image->width; j++)
        {
            // Calculer les nouvelles coordonnées après translation
            int newX = (j + x) % image->width;
            int newY = (i + y) % image->height;

            // Assigner la valeur de pixel à la nouvelle position
            newPixels[newY * image->width + newX] = image->pixels[i * image->width + j];
        }
    }

    // Libérer l'ancien tableau de pixels et mettre à jour l'image avec le nouveau tableau
    free(image->pixels);
    image->pixels = newPixels;
}

void blurImage(struct GrayImage *image, int blurPercent)
{
    int blurSize = blurPercent * 0.01 * (image->width + image->height) / 2; // Calculer la taille du flou
    unsigned char *newPixels = (unsigned char *)malloc(image->width * image->height * sizeof(unsigned char));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int total = 0;
            int count = 0;
            // Calculer la moyenne des pixels voisins
            for (int dy = -blurSize; dy <= blurSize; dy++)
            {
                for (int dx = -blurSize; dx <= blurSize; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;
                    // Vérifier les limites de l'image
                    if (nx >= 0 && nx < image->width && ny >= 0 && ny < image->height)
                    {
                        total += image->pixels[ny * image->width + nx];
                        count++;
                    }
                }
            }
            newPixels[y * image->width + x] = total / count;
        }
    }

    free(image->pixels);
    image->pixels = newPixels;
}

void pixelateImage(struct GrayImage *image, int pixelatePercent)
{
    int blockSize = pixelatePercent * 0.01 * (image->width + image->height) / 2; // Calculer la taille du bloc
    unsigned char *newPixels = (unsigned char *)malloc(image->width * image->height * sizeof(unsigned char));

    for (int y = 0; y < image->height; y += blockSize)
    {
        for (int x = 0; x < image->width; x += blockSize)
        {
            int total = 0;
            int count = 0;
            // Calculer la moyenne des pixels dans le bloc
            for (int dy = 0; dy < blockSize; dy++)
            {
                for (int dx = 0; dx < blockSize; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < image->width && ny < image->height)
                    {
                        total += image->pixels[ny * image->width + nx];
                        count++;
                    }
                }
            }
            unsigned char blockAverage = total / count;
            // Assigner la moyenne à tous les pixels dans le bloc
            for (int dy = 0; dy < blockSize; dy++)
            {
                for (int dx = 0; dx < blockSize; dx++)
                {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx < image->width && ny < image->height)
                    {
                        newPixels[ny * image->width + nx] = blockAverage;
                    }
                }
            }
        }
    }

    free(image->pixels);
    image->pixels = newPixels;
}

void scaleImage(struct GrayImage *image, double scaleFactor)
{
    int newWidth = (int)(image->width * scaleFactor);
    int newHeight = (int)(image->height * scaleFactor);
    unsigned char *newPixels = (unsigned char *)malloc(newWidth * newHeight * sizeof(unsigned char));

    if (newPixels == NULL)
    {
        perror("Erreur d'allocation mémoire pour newPixels");
        exit(1);
    }

    for (int y = 0; y < newHeight; y++)
    {
        for (int x = 0; x < newWidth; x++)
        {
            int origX = (int)(x / scaleFactor);
            int origY = (int)(y / scaleFactor);
            newPixels[y * newWidth + x] = image->pixels[origY * image->width + origX];
        }
    }

    free(image->pixels);
    image->width = newWidth;
    image->height = newHeight;
    image->pixels = newPixels;
}

void invertColors(struct GrayImage *image)
{
    for (int i = 0; i < image->width * image->height; i++)
    {
        image->pixels[i] = 255 - image->pixels[i];
    }
}

//------------------------------------------------------------------------------------------------------------------
//
// - Misc
//
//------------------------------------------------------------------------------------------------------------------

int showPPMMenu()
{
    int choice;

    printf("Menu de traitement d'image PPM:\n");
    printf("-------------------------------\n");
    printf("\n");
    printf("Ce programme ne supporte pas les images PPM");
    printf("\n");
    printf("-------------------------------\n");
    printf("\n");
    printf("Choisissez une option : ");
    scanf("%d", &choice);

    return choice;
}

int showPGMMenu()
{
    int choice;

    printf("Menu de traitement d'image PGM:\n");
    printf("-------------------------------\n");
    printf("1. Charger une image\n");
    printf("2. Appliquer une translation\n");
    printf("3. Appliquer un effet : Rotation\n");
    printf("4. Appliquer un effet : Miroir Horizontal\n");
    printf("5. Ajuster le contraste\n");
    printf("6. Ajuster la luminosité\n");
    printf("7. Appliquer le seuillage\n");
    printf("8. Appliquer un flou\n");
    printf("9. Appliquer une pixelisation\n");
    printf("10. Inverser les couleurs\n");
    printf("11. Quitter\n");
    printf("-------------------------------\n");
    printf("\n");
    printf("Choisissez une option : ");

    scanf("%d", &choice);

    return choice;
}

int demanderTypeFichier()
{
    int type;
    printf("je suis un lol oui");
    printf("S'agit-il d'un fichier ppm ou pgm: 1 = pgm, 2 = ppm ");
    scanf("%d", &type);
    return type;
}

//------------------------------------------------------------------------------------------------------------------
//
// - Main
//
//------------------------------------------------------------------------------------------------------------------

int main()
{
    struct GrayImage grayscaleImage;
    int choice, typeFichier;
    char inputFilename[100];
    char outputFilename[100];
    int blurPercent;
    int pixelatePercent;

    typeFichier = demanderTypeFichier(); // Demander le type de fichier

    do
    {
        if (typeFichier == 1)
        {
            choice = showPGMMenu(); // Affiche le menu PGM
        }
        else if (typeFichier == 2)
        {
            choice = showPPMMenu(); // Affiche le menu PPM
        }
        else
        {
            printf("Type de fichier non valide. Veuillez redémarrer le programme.\n");
            break;
        }

        switch (choice)
        {
        case 1: // loader
            printf("Entrez le nom du fichier à charger (avec extension) : ");
            scanf("%s", inputFilename);
            grayscaleImage = readPGM(inputFilename);
            printf("Image chargée avec succès.\n");
            break;
        case 2: // translation
            printf("Entrez le décalage en X : ");
            int x;
            scanf("%d", &x);

            printf("Entrez le décalage en Y : ");
            int y;
            scanf("%d", &y);

            translateImage(&grayscaleImage, x, y);
            printf("Translation appliquée avec succès.\n");

            printf("Entrez le nom du fichier de sortie (avec extension) : ");
            scanf("%s", outputFilename);
            writePGM(&grayscaleImage, outputFilename);
            printf("Image enregistrée avec succès dans %s.\n", outputFilename);
            free(grayscaleImage.pixels);
            break;
        case 3: // rotation
            printf("Entrez l'angle de rotation (en degrés) : ");
            int angle;
            scanf("%d", &angle);
            rotateImage(&grayscaleImage, angle);
            printf("Rotation appliquée avec succès.\n");
            break;

        case 4: // miroir
            mirrorHorizontal(&grayscaleImage);
            printf("Effet miroir appliqué avec succès.\n");
            break;
        case 5: // constraste
            printf("Entrez le facteur de contraste (1.0 pour aucun changement) : ");
            double contrastFactor;
            scanf("%lf", &contrastFactor);
            adjustContrast(&grayscaleImage, contrastFactor);
            printf("Contraste ajusté avec succès.\n");
            break;
        case 6: // luminosité
            printf("Entrez la valeur de luminosité (-255 à 255) : ");
            int brightnessValue;
            scanf("%d", &brightnessValue);
            adjustBrightness(&grayscaleImage, brightnessValue);
            printf("Luminosité ajustée avec succès.\n");
            break;
        case 7: // seuillage
            printf("Entrez la valeur de seuil (0 à 255) : ");
            int thresholdValue;
            scanf("%d", &thresholdValue);
            applyThresholding(&grayscaleImage, thresholdValue);
            printf("Seuillage appliqué avec succès.\n");
            break;
        case 8: // Floutage
            do
            {
                printf("Entrez les niveaux de floutage (1-5) : ");
                scanf("%d", &blurPercent);
                if (blurPercent < 1 || blurPercent > 5)
                {
                    printf("Veuillez entrer un nombre entre 1 et 5.\n");
                }
            } while (blurPercent < 1 || blurPercent > 5);
            blurImage(&grayscaleImage, blurPercent);
            printf("Floutage appliqué avec succès.\n");
            break;

        case 9: // Pixelisation
            do
            {
                printf("Entrez les niveaux de pixelisation (1-5) : ");
                scanf("%d", &pixelatePercent);
                if (pixelatePercent < 1 || pixelatePercent > 5)
                {
                    printf("Veuillez entrer un nombre entre 1 et 5.\n");
                }
            } while (pixelatePercent < 1 || pixelatePercent > 5);
            pixelateImage(&grayscaleImage, pixelatePercent);
            printf("Pixelisation appliquée avec succès.\n");
            break;
        case 10:
            invertColors(&grayscaleImage);
            printf("Inversion de l'image éffectuée avec succès");
            break;
        /*case 10:
            printf("Entrer un facteur d'echelle (double: -1+)\n");
            double scaleFactor;
            scanf("%lf", &scaleFactor);
            scaleImage(&grayscaleImage, scaleFactor);
            printf("Mise à échelle éfféctuée avec succès.\n");
            break;*/
        case 11:
            printf("c'est chaoo\n");
            break;
        default:
            printf("Option non valide. Veuillez choisir une option valide.\n");
        }

        if (choice > 2 && choice < 10)
        {
            printf("Entrez le nom du fichier de sortie (avec extension) : ");
            scanf("%s", outputFilename);
            writePGM(&grayscaleImage, outputFilename);
            printf("Image enregistrée avec succès dans %s.\n", outputFilename);
            free(grayscaleImage.pixels);
        }

    } while (choice != 11);

    return 0;
}

// scaling, histogramme
