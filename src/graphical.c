#include "graphical.h"

//taille du terminal
tab_size_t termsize;
//settings du terminal (a réstaurer lors de la fin du jeu)
struct termios orig_term_settings;
//pour savoir si on a besoin de dire au terminal de changer sa couleur d'affichage ou pas
//(eviter de le faire a chaque char si ils sont de la mème couleure)
COLOR cur_color, cur_color_background;
//buffer pour la sortie d'affichage (permet un affichage plus rapide, mais demande de fflush a la main lors des affichage)
char stdout_buffer[100];
//stockage des pixels utilisée par le compositeur
//c'est un tableau 3D (5 images les unes a la suite des autres)
//chaque image 2D a pour taille termsize, et l'éspacement entre chaque images est stocké dans compositor_stride
pixel_t *compositor_pixels;
uint compositor_stride;


void    graphical_cleanup(void)
{
    //appellé a la sortie, résponsable de clean les graphisme et de réstaurer le terminal

    //restore les attributs du terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term_settings);
    //réaffiche le curseur, et reset la couleur d'affichage au valeur par défault
    printf("%s", "\33[H \33[?25h \33[0m \33[2J");
	
	//free les variables du compositeur
    compose_free();
}

void    init_graphical(void)
{
    //*mets en place les graphismes*
	
    //mets en place le buffering
    setvbuf(stdout, stdout_buffer, _IOFBF, 100);
    //cache le curseur
    printf("%s", "\33[?25l");
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

	//VTIME et VMIN permettent a scanf (read) de retourner instantanément 0 si l'utilisateur n'a rien rentré
	//et de fonctionner normalement sinon
    raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0;
    //applique tout les changements
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    #endif //SYSTEM_POSIX

    //setup windows, non testé, par manque de machine windows a disposition
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

    //set la langue a rien, pour que certaint settings de langue bizarre
	//(ex langue de droite a gauche) n'influent pas sur le programme
    setlocale(LC_ALL, "");
    //set la valeur par default de certaine variables globales
    cur_color            = COL_DEFAULT;
    cur_color_background = COL_DEFAULT;
	//initialise le compositeur
    compose_init();
}


/*****************************************
 *** DIRECT PICTURES&TERMINAL UTILITIES ***
 ******************************************/


bool    pix_equal(pixel_t pix1, pixel_t pix2)
{
    //compare les charactères
	//
    if (pix1.c1!=pix2.c1)
        return false;                   //pixels différents
    if (pix1.c1=='\0')
        return true;                //pixels transparents: on ne check pas la couleure
    if (pix1.c2!=pix2.c2)
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
    if (pix1.color==pix2.color && pix1.background_color==pix2.background_color)
    {
        return true;
    }
	//si c'est un espace, on ne check que le background color
	if (pix1.c1==' ' && pix1.background_color==pix2.background_color) {
		return true;
	}
    //touts les test sont passés: les 2 pixels sont égaux
    return false;
}

picture_t    pict_crop_bound(picture_t pict, coordonee_t min, coordonee_t max)
{
    //coupe les cotés d'une image, sans copier les données
    //(une modification de cette image modifiera l'image de départ)
    return (picture_t)
           {
               .size =
               {
                   .col    = max.x - min.x + 1,
                   .row    = max.y - min.y + 1,
                   .stride = pict.size.stride
               },
               .data = &pict.data[offset_of(min, pict.size.stride)]
           };
}

picture_t    pict_crop_size(picture_t pict, coordonee_t min, coordonee_t size)
{
    //coupe les cotés d'une image, sans copier les données
    return (picture_t)
           {
               .size =
               {
                   .col    = size.x,
                   .row    = size.y,
                   .stride = pict.size.stride
               },
               .data = &pict.data[offset_of(min, pict.size.stride)]
           };
}

void    go_to(coordonee_t    pos)
{
    //mets le curseur a la position demandée sur le terminal
    //le terminal compte a partir de 1, C a partir de 0, donc on ajoute 1
    printf("\33[%i;%if", pos.y + 1, pos.x + 1);
}
void    advance_cursor(uint    nb)
{
    //avance le cusreur de nb charctères
    if (nb>0)
    {
        printf("\33[%uC", nb);
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
            printf("\33[39m");
        }
        else
        {
            printf("\33[38;5;%im", color);
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
            printf("\33[49m");
        }
        else
        {
            printf("\33[48;5;%im", color);
        }
    }
}

void    pict_direct_display(picture_t pict, coordonee_t pos)
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
                    go_to(
                        (coordonee_t){ .x = j + pos.x, .y = i + pos.y }
                        );
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
//mets en forme le texte dans une image
//NB: ne supporte malheuresement pas l'utf-8
void    txt_to_img(picture_t result, const char *text_to_display, COLOR text_color, COLOR background_color)
{
    pixel_t pixel_to_diplay =
    {
        .c2               = '\0',
        .background_color = background_color,
        .color            = text_color
    };
    coordonee_t pos;
    int32_t counter = 0;
    for (pos.y = 0; pos.y < result.size.row; pos.y++)
    {
        for (pos.x = 0; pos.x < result.size.col; pos.x++)
        {
            char char_to_print = text_to_display[counter];
            counter++;
            if (char_to_print=='\0')
            {
                //on a atteint la fin de la chaine de charactère
                return;
            }
            else if (char_to_print == '\n')
            {
                //on passe a la ligne suivante
                break;
            }
            pixel_to_diplay.c1                              = char_to_print;
            result.data[offset_of(pos, result.size.stride)] = pixel_to_diplay;
        }
    }
}


/***************************
 *** COMPOSITOR UTILITIES ***
 ****************************/


//initialise le compositeur
void    compose_init(void)
{
    compositor_stride = termsize.col * termsize.row;
    //on demande 5 images de la taille de l'écran, se suivant en mémoire
    //l'écart (en nombre de pixels) et donc de compositor_stride (= nombre de pixels dans une image = col * row)
    compositor_pixels = (pixel_t *)safe_malloc(sizeof(pixel_t) * compositor_stride * 5);
}

//free le compositeur
void    compose_free(void)
{
    free(compositor_pixels);
    compositor_stride = 0;
}

void    compose_disp_rect(COLOR color, COMPOSE_RANK rank, coordonee_t pos, coordonee_t size)
{
    pixel_t pix_color =
    {
        .color = COL_DEFAULT,
        .background_color = color,
        .c1    = ' ',
        .c2    = '\0',
    };

    for (int i = 0; i<size.y; i++)
    {
        for(int j = 0; j < size.x; j++)
        {
            compose_disp_pix(
                pix_color,
                rank,
                (coordonee_t){
                    .x = j + pos.x,
                    .y = i + pos.y
                }
                );
        }
    }
}

//donne une image au compositeur
void    compose_disp_pict(picture_t pict, COMPOSE_RANK rank, coordonee_t pos)
{
    for (int i = 0; i<pict.size.row; i++)
    {
        //on copie l'image a l'aide de memcpy (copie par ligne entière)
        //dans la mémoire du compositeur

        //                        colonne        ligne           rang (profondeur) de l'image
        memcpy(
            &compositor_pixels[pos.x + (i + pos.y) * termsize.stride + rank * compositor_stride], //destination
            &pict.data[i * pict.size.stride],                                                //source
            sizeof(pixel_t) * pict.size.col
            );                                                                               //nombre d'octets a copier
        //puis on calcule les changements éventuels
        for (int j = pos.x; j<pos.x + pict.size.col; j++)
        {
            compose_have_changed(
                (coordonee_t){ j, i + pos.y }
                );
        }
    }
}

void    compose_disp_text(const char *text_to_display, COLOR text_color, COLOR background_color, COMPOSE_RANK rank, coordonee_t pos, coordonee_t size_of_text_box)
{
    //getting location
    picture_t text_image =
    {
        .size = termsize, .data = &compositor_pixels[compositor_stride * rank]
    };
    text_image = pict_crop_size(text_image, pos, size_of_text_box);
    txt_to_img(text_image, text_to_display, text_color, background_color);

    for (int y=0; y<size_of_text_box.y; y++) {
		for (int x=0; x<size_of_text_box.x; x++) {
			compose_have_changed((coordonee_t){pos.x+x,pos.y+y});
		}
    }
}
//donne un seul pixel au compositeur
void    compose_disp_pix(pixel_t pixel, COMPOSE_RANK rank, coordonee_t pos)
{
    compositor_pixels[pos.x + pos.y * termsize.stride + rank * compositor_stride] = pixel;
    compose_have_changed(pos);
}

//calcule les changements a la position demandée
void    compose_have_changed(coordonee_t    pos)
{
    //emplacement du pixel dans une image
    int offshet = offset_of(pos, termsize.stride);

    //calcul du nouveau pixel
    COMPOSE_RANK rank = COMPOSE_UI;
    pixel_t pixel     =
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
        if (compositor_pixels[ offshet + compositor_stride * rank ].c1 != '\0') //si le pixel n'est pas transparent, on prend sa couleur de background
            pixel.background_color = compositor_pixels[ offshet + compositor_stride * rank ].background_color;
        rank += 1;
    }
    //si le pixel a changé, on l'ajoute au changements et on le modifie
    if ( !pix_equal(pixel, compositor_pixels[ offshet + compositor_stride * COMPOSE_RESULT ]) )
    {
        compositor_pixels[ offshet + compositor_stride * COMPOSE_CHANGES ] = pixel;
    } //sinon, on le retire des changements
    else
    {
        compositor_pixels[ offshet + compositor_stride * COMPOSE_CHANGES ].c1 = '\0';
    }
}
void    compose_refresh(void)
{
    picture_t changes =
    {
        .size = termsize,
        .data = &compositor_pixels[compositor_stride * COMPOSE_CHANGES],
    };
    pict_direct_display(
        changes,
        (coordonee_t){ 0, 0 }
        );
    fflush(stdout);

    //on update COMPOSE_RESULT avec les changements
    int32_t offset;
    pixel_t pixel;

    for (int32_t x = 0; x<termsize.col; x++)
    {
        for (int32_t y = 0; y<termsize.row; y++)
        {
            offset = x + termsize.col * y;
            pixel  = compositor_pixels[ offset + compositor_stride * COMPOSE_CHANGES ];
            //si le pixel a changé
            if (pixel.c1!='\0')
            {
                compositor_pixels[ offset + compositor_stride * COMPOSE_RESULT]     = pixel;
                compositor_pixels[ offset + compositor_stride * COMPOSE_CHANGES].c1 = '\0';
            }
        }
    }
}
//efface un pixel
void    compose_del_pix(COMPOSE_RANK rank, coordonee_t pos)
{
    compositor_pixels[pos.x + pos.y * termsize.stride + rank * compositor_stride].c1 = '\0';
    compose_have_changed(pos);
}
void    compose_del_area(COMPOSE_RANK rank, coordonee_t min, coordonee_t max)
{
    for (int x = min.x; x<=max.x; x++)
    {
        for (int y = min.y; y<=max.y; y++)
        {
            compose_del_pix(
                rank,
                (coordonee_t){ x, y }
                );
        }
    }
}

