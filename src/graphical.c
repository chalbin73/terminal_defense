#include "graphical.h"


tab_size_t termsize;
struct termios orig_term_settings;
COLOR cur_color, cur_color_background;
char stdout_buffer[100];
//stockage des pixels utilisée par le compositeur
//c'est un tableau 3D (images (sans la taille) les unes a la suite des autres)
//chaque image 2D a pour taille termsize, et l'éspacement entre chaque images est stocké dans compositor_stride
pixel_t *compositor_pixels;
uint compositor_stride;


const char CHARS_BLOCK_UP[4]       = "▀";
const char CHARS_BLOCK_DOWN[4]     = "▄";
const char CHARS_BLOCK_FULL[4]     = "█";
const char CHARS_BLOCK_LEFT[4]     = "▌";
const char CHARS_BLOCK_RIGHT[4]    = "▐";
const char CHARS_BLOCK_LIGHT[4]    = "░";
const char CHARS_BLOCK_MEDIUM[4]   = "▒";
const char CHARS_BLOCK_DARK[4]     = "▓";
const char CHARS_TRIANGLE_LEFT[4]  = "◀";
const char CHARS_TRIANGLE_RIGHT[4] = "▶";
const char CHARS_TRIANGLE_DOWN[4]  = "▼";
const char CHARS_TRIANGLE_UP[4]    = "▲";


void    graphical_cleanup()
{
    //appellé a la sortie, résponsable de clean les graphisme

    //restore les attributs du terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term_settings);
    //réaffiche le curseur, et reset la couleur
    printf("%s", "\e[H \e[?25h \e[0m \e[2J");

    compose_free();

}

void    init_graphical()
{
    //mets en place les graphismes
    //mets en place le buffering
    setvbuf(stdout, stdout_buffer, _IOFBF, 100);
    //maximise le terminal et cache le curseur
    printf("%s", "\e[?25l \e[8;99999;99999t");

    //obtention de la taille de l'écran
    struct winsize raw_termsize;
    ioctl(0, TIOCGWINSZ, &raw_termsize);
    termsize.col    = raw_termsize.ws_col;
    termsize.row    = raw_termsize.ws_row;
    termsize.stride = raw_termsize.ws_col; //utilisé pour pouvoir assigner rappidement la taille de l'écran a des images

    //setup du terminal sur linux
    #if SYSTEM_POSIX
    //sauvegarde les attributs terminal (pour restauration future) et mets le term en raw mode
    //Voir http://manpagesfr.free.fr/man/man3/termios.3.html pour plus de détails
    tcgetattr(STDIN_FILENO, &orig_term_settings);
    struct termios raw = orig_term_settings;
    raw.c_lflag   &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    raw.c_iflag   &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag   &= ~(OPOST);
    raw.c_cflag   &= ~(CSIZE | PARENB);
    raw.c_cflag   |= CS8;
    raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0;     //VTIME a 1 permet de timeout scanf au bout de 0.1 secondes (jeu a 10 fps)
    //tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    #endif //SYSTEM_POSIX

    //setup windows
    #if SYSTEM_WINDOWS
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        printf("Terminal setup failed, please use a compatible terminal");
        exit(128);
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        printf("Terminal setup failed, please use a compatible terminal");
        exit(128);
    }

    DWORD dwOriginalOutMode = 0;
    DWORD dwOriginalInMode  = 0;
    if ( !GetConsoleMode(hOut, &dwOriginalOutMode) )
    {
        printf("Terminal setup failed, please use a compatible terminal");
        exit(128);
    }
    if ( !GetConsoleMode(hIn, &dwOriginalInMode) )
    {
        printf("Terminal setup failed, please use a compatible terminal");
        exit(128);
    }

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD dwRequestedInModes  = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if ( !SetConsoleMode(hOut, dwOutMode) )
    {
        // we failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode           = dwOriginalOutMode | dwRequestedOutModes;
        if ( !SetConsoleMode(hOut, dwOutMode) )
        {
            // Failed to set any VT mode, can't do anything here.
            printf("Terminal setup failed, please use a compatible terminal");
            exit(128);
        }
    }

    DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
    if ( !SetConsoleMode(hIn, dwInMode) )
    {
        // Failed to set VT input mode, can't do anything here.
        printf("Terminal setup failed, please use a compatible terminal");
        exit(128);
    }

    #endif //SYSTEM_WINDOWS

    //set terminal locale to make behavior more robust accros different settings
    setlocale(LC_ALL, "");
    //set la valeur par default de certaine variables globales
    cur_color            = COL_DEFAULT;
    cur_color_background = COL_DEFAULT;
    compose_init();
    //chaine vide (des 0) pour l'input_string
}


/*****************************************
 *** DIRECT PICTURES&TERMINAL UTILITIES ***
 ******************************************/


bool    pix_equal(pixel_t pix1, pixel_t pix2)
{
    //compare les charactères
    if (pix1.c1!=pix2.c2)
        return false;                   //pixels différents
    if (pix1.c1=='\0')
        return true;                //pixels transparents: on ne check pas la couleure
    if (pix1.c2!=pix2.c3)
        return false;                   //on check le 2 ème char
    if (pix1.c2!='\0')  //si plus d'un char est utilisés (UTF-8 sur 2-4 char)
    {
        if (pix1.c3!=pix2.c3)
            return false;
        if (pix1.c3!='\0')  //si plus de 2 char sont utilisés (UTF-8 sur 3 ou 4 char)
        {
            if (pix1.c4!=pix2.c4)
                return false;
        }
    }
    //si les char sont égaux, on check les couleurs
    if (pix1.color!=pix2.color || pix1.background_color!=pix2.background_color)
    {
        return false;
    }
    //touts les test sont passés: les 2 pixels sont égaux
    return true;
}


picture_t    pict_crop_bound(picture_t pict, uint xmin, uint xmax, uint ymin, uint ymax)
{
    //coupe les cotés d'une image, sans copier les données
    //(une modification de cette image modifiera l'image de départ)
    return (picture_t)
           {
               .size =
               {
                   .col    = xmax - xmin + 0,
                   .row    = ymax - ymin + 1,
                   .stride = pict.size.stride
               },
               .data = pict.data + xmin + pict.size.stride * ymin
           };
}
picture_t    pict_crop_size(picture_t pict, uint xmin, uint col, uint ymin, uint row)
{
    //coupe les cotés d'une image, sans copier les données
    return (picture_t)
           {
               .size =
               {
                   .col    = col,
                   .row    = row,
                   .stride = pict.size.stride
               },
               .data = pict.data + xmin + pict.size.stride * ymin
           };
}

void    go_to(int ligne, int colonne)
{
    //mets le curseur a la position demandée sur le terminal
    //le terminal compte a partir de 1, C a partir de 0, donc on ajoute 1
    printf("\e[%i;%if", ligne + 1, colonne + 1);
}
void    advance_cursor(uint    nb)
{
    //avance le cusreur de nb charctères
    if (nb>0)
    {
        printf("\e[%uC", nb);
    }
}
void    set_color(int    color)
{
    //si la couleur est différente de l'actuelle
    //set la nouvelle couleur a l'aide de carctères d'échapement
    if (color != cur_color)
    {
        cur_color = color;
        if (color==-1)
        {
            printf("\e[39m");
        }
        else
        {
            printf("\e[38;5;%im", color);
        }
    }
}
void    set_color_background(int    color)
{
    //si la couleur est différente de l'actuelle
    //set la nouvelle couleur a l'aide de carctères d'échapement
    if (color != cur_color_background)
    {
        cur_color_background = color;
        if (color==-1)
        {
            printf("\e[49m");
        }
        else
        {
            printf("\e[48;5;%im", color);
        }
    }
}

void    pict_direct_display(picture_t pict, uint ligne, uint colonne)
{
    //affiche sur le terminal une image a une position donnée

    //optimisé pour un grand nombre de \0 (a ne pas afficher)

    //pour chaque ligne
    for (int i = 0; i<pict.size.row; i++)
    {
        //sert a compter le nombre de charactères depuis le derniers affichage
        //vaut -1 en cas de changement de ligne
        int ecart = -1;
        //pour chaque colonne
        for (int j = 0; j<pict.size.col; j++)
        {
            //on obtient le pixel a afficher
            pixel_t *pixel = &pict.data[j + i * pict.size.stride];
            //si il est nul, on l'ignore
            if (pixel->c1=='\0')
            {
                //si il y a un écart, on l'augmente
                if (ecart>=0)
                {
                    ecart += 1;
                }
            }
            else     //le pixel n'est pas nul, on l'affiche
            {
                if (ecart==-1)
                {
                    //on a changé de ligne depuis le dernier char affiché
                    go_to(i + ligne, j + colonne);
                }
                else
                {
                    //on est a la bonne ligne, il suffit de se décaler
                    advance_cursor(ecart);
                }
                ecart = 0;
                set_color(pixel->color);
                set_color_background(pixel->background_color);
                printf( "%.4s", &(pixel->c1) );
            }
        }
    }
}


/***************************
 *** COMPOSITOR UTILITIES ***
 ****************************/


//initialise le compositeur
void    compose_init()
{
    compositor_stride = termsize.col * termsize.row;
    //on demande 5 images de la taille de l'écran, se suivant en mémoire
    //l'écart (en nombre de pixels) et donc de compositor_stride (= nombre de pixels dans une image = col * row)
    compositor_pixels = (pixel_t *)safe_malloc(sizeof(pixel_t) * compositor_stride * 5);
}

//free le compositeur
void    compose_free()
{
    free(compositor_pixels);
    compositor_stride = 0;
}
//donne une image au compositeur
void    compose_disp_pict(picture_t pict, COMPOSE_RANK rank, uint posx, uint posy)
{
    for(int y = 0; y < pict.size.col; y++)
    {
        for(int x = 0; x < pict.size.row; x++)
        {
            compose_disp_pix(pict.data[(y * pict.size.stride) + x], rank, posx + x, posy + y);
        }
    }
}

/*{
    for (int i=0; i<pict.size.row; i++){
        //on copie l'image a l'aide de memcpy (copie par ligne entière)
        //dans la mémoire du compositeur

        //                        colonne        ligne           rang (profondeur) de l'image
        memcpy(&compositor_pixels[posx + (i+posy)*termsize.stride + rank*compositor_stride], //destination
               &pict.data[i*pict.size.stride],                                               //source
               sizeof(pixel_t)*pict.size.col);                                               //nombre d'octets a copier
        //puis on calcule les changements éventuels
        for (int j=posx; j<posx+pict.size.col; j++){
            compose_have_changed(i+posy,j);
        }
    }
   }*/
//donne un seul pixel au compositeur
void    compose_disp_pix(pixel_t pixel, COMPOSE_RANK rank, uint posx, uint posy)
{
    compositor_pixels[posx + posy * termsize.stride + rank * compositor_stride] = pixel;
    compose_have_changed(posx, posy);
}

//calcule les changements a la position demandée
void    compose_have_changed(int posx, int posy)
{
    //emplacement du pixel dans une image
    int offshet = posx + posy * termsize.stride;

    //calcul du nouveau pixel
    int rank      = COMPOSE_UI;
    pixel_t pixel =
    {
        .c1 = '\0'
    };
    //on obtient le premier pixel non-nul
    while (rank<=COMPOSE_BACK && pixel.c1=='\0')
    {
        pixel = compositor_pixels[ offshet + compositor_stride * rank ];
        rank += 1;
    }
    //si la couleur de fond et par défault, on va chercher celle des pixels en dessous
    while (rank<=COMPOSE_BACK && pixel.background_color==COL_DEFAULT)
    {
        pixel.background_color = compositor_pixels[ offshet + compositor_stride * rank ].background_color;
    }
    //si le pixel a changé, on l'ajoute au changements et on le modifie
    if ( !pix_equal(pixel, compositor_pixels[ offshet + compositor_stride * COMPOSE_RESULT ]) )
    {
        compositor_pixels[ offshet + compositor_stride * COMPOSE_CHANGES ] = pixel;
        compositor_pixels[ offshet + compositor_stride * COMPOSE_RESULT ]  = pixel;
    }
}
void    compose_refresh()
{
    picture_t changes =
    {
        .size = termsize,
        .data = &compositor_pixels[compositor_stride * COMPOSE_CHANGES],
    };
    pict_direct_display(changes, 0, 0);
    fflush(stdout);
    //on reset les changements (puisqu' on vient de les afficher)
    memset( changes.data, 0, compositor_stride * sizeof(pixel_t) );

}
//efface un pixel
void    compose_del_pix(COMPOSE_RANK rank, int posx, int posy)
{
    compositor_pixels[posx + posy * termsize.stride + rank * compositor_stride].c1 = '\0';
    compose_have_changed(posx, posy);
}
void    compose_del_area(COMPOSE_RANK rank, int minx, int maxx, int miny, int maxy)
{
    for (; minx<=maxx; minx++)
    {
        for (; miny<=maxy; miny++)
        {
            compose_del_pix(rank, minx, miny);
        }
    }
}

