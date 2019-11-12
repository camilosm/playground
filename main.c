#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>



enum CAMERAS { GERAL = 1, BRINQUEDOS, PRIMEIRA};
enum BRINQUEDO { RODA = 1, CARROSSEL, QUEDA, XICARA};
int modoCAM = GERAL;            //variável responsável por guardar o modo de câmera que está sendo utilizado
int foco = RODA;

int xMouse = 0, yMouse = 0;     //variáveis globais que serão usadas na função posicionaCamera
int xCursor, yCursor, zCursor;  //guarda o centro do cursor
float phi = 90, teta = 0;       //ângulos das coordenadas esféricas
float anguloRoda=0, posicaoQueda=0, vQueda;
int light=0;

unsigned int texGround;

// estrutura de dados que representará as coordenadas da câmera
struct {
  float x, y, z;
  float targetX, targetY, targetZ;
} camera;

unsigned int carregarTextura(char* arquivo){
    unsigned int id = SOIL_load_OGL_texture(arquivo, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if(!id)
        printf("Erro carregando a textura '%s': '%s'\n", arquivo, SOIL_last_result());
    return id;
}

void tocar_musica(char const nome[40], int loop) {
    Mix_Chunk *som = NULL;
    int canal;
    int canal_audio = 2;
    int taxa_audio = 22050;
    Uint16 formato_audio = AUDIO_S16SYS;
    int audio_buffers = 4096;

    if(Mix_OpenAudio(taxa_audio, formato_audio, canal_audio, audio_buffers)){
        printf("Falha ao tocar musica: %s\n", Mix_GetError());
				return;
    }

    som = Mix_LoadWAV(nome);

    if(som == NULL){
        printf("Falha ao tocar musica: %s\n", Mix_GetError());
				return;
    }

    Mix_HaltChannel(-1);
    canal = Mix_PlayChannel(-1, som, loop);

    if (canal == -1) {
        printf("Falha ao tocar musica: %s\n", Mix_GetError());
				return;
    }
}


void teclado(unsigned char key, int x, int y) {
    switch (key) {
        case 27:    //aperte ESC para fechar
            exit(0);
            break;
        case 's':   //andar pelo plano X-Z utilizando W A S D
            zCursor++;
            break;
        case 'w':
            zCursor--;
            break;
        case 'a':
            xCursor--;
            break;
        case 'd':
            xCursor++;
            break;
        case '1':
            modoCAM = GERAL;
            break;
        case '2':
            modoCAM = BRINQUEDOS;
            break;
				case '3':
            modoCAM = PRIMEIRA;
            break;
				case 'l':
						if(light)
							light--;
						else
							light++;
						break;
        default:
            break;
    }
}

void special(int key, int x, int y){
	switch(key){
		case GLUT_KEY_RIGHT:
			foco++;
			if(foco>XICARA)
				foco=RODA;
			break;
		case GLUT_KEY_LEFT:
			foco--;
			if(foco<RODA)
				foco=XICARA;
			break;
	}
}

//capturar posicionamento do mouse
void posicionaCamera(int x, int y){
    // variáveis que guardam o vetor 2D de movimento do mouse na tela
    // xMouse e yMouse são os valores de x e y no frame anterior
    float xChange = x - xMouse;
    float yChange = y - yMouse;

    // este exemplo usa coordenadas esféricas para controlar a câmera...
    // teta e phi guardam a conversão do vetor 2D para um espaço 3D
    // com coordenada esférica
    teta = (teta + xChange/150);
    phi = (phi - yChange/150);

    if(phi >= 180){
      //limite de 180 para o phi
      phi = 180;
    }

    // guarda o x e y do mouse para usar na comparação do próximo frame
    xMouse = x;
    yMouse = y;
}


// callback de atualização
void atualiza(int time) {
    anguloRoda+=0.5;
		if(anguloRoda>360)
			anguloRoda=0;
		if(posicaoQueda>=50)
			vQueda=-1;
		if(posicaoQueda<=5)
			vQueda=0.2;
		posicaoQueda+=vQueda;

		glutPostRedisplay();
    glutTimerFunc(time, atualiza, time);
}

void redimensiona(int w, int h){
    glEnable(GL_DEPTH_TEST);                // Ativa o Z buffer
    glViewport (0, 0, w, h);                //define a proporção da janela de visualização
    glMatrixMode (GL_PROJECTION);           //define o tipo de matriz de transformação que será utilizada
    glLoadIdentity();                       //carrega a matriz identidade do tipo GL_PROJECTION configurado anteriormente
    gluPerspective(60.0, (float)w/(float)h, 0.2, 400.0);    //funciona como se fosse o glOrtho, mas para o espaço 3D
    glMatrixMode(GL_MODELVIEW);                             //ativa o modo de matriz de visualização para utilizar o LookAt
}

void inicializa(){
    glClearColor(1, 1, 1, 1);                          //cor de fundo branca
    glEnable(GL_BLEND);                                //ativa a mesclagem de cores
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //ativando o blend, podemos criar objetos transparentes
    xCursor = 0;                                       //a câmera começa olhando para o ponto 0
    yCursor = 0;
    zCursor = 0;

		texGround=carregarTextura("ground.png");

		tocar_musica("music.ogg", -1);
}

void desenhaChao(){
	glColor3f(1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(1,1); glVertex3f(100, 0, 100);
		glTexCoord2f(0,1); glVertex3f(-100, 0, 100);
		glTexCoord2f(0,0); glVertex3f(-100, 0, -100);
		glTexCoord2f(1,0); glVertex3f(100, 0, -100);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void desenhaArvore(float x, float y, float z){
	glColor3f((float)66/255, (float)99/255, (float)81/255);
	glPushMatrix();
		glTranslatef(x, y, z);
		glRotatef(90, -1, 0, 0);
		glutSolidCone(5, 10, 8, 8);
		glTranslatef(0, 0, 7);
		glutSolidCone(4, 8, 8, 8);
		glTranslatef(0, 0, 5);
		glutSolidCone(3, 6, 8, 8);
	glPopMatrix();
}

void desenhaFloresta(){
	int i;
	for(i=-95;i<=95;i+=7){
		desenhaArvore(-95,0,i);
		desenhaArvore(95,0,i);
		desenhaArvore(i,0,-95);
		desenhaArvore(i,0,95);
	}
}

void desenhaRoda(float angulo){
	glColor3f(0.7,0.7,0.7);
	glPushMatrix();
		glPushMatrix();
			glutSolidCube(15);
		glPopMatrix();
		glTranslatef(0, 10, 0);
		glRotatef(angulo, 0, 0, 1);
		glColor3f(0.5,0.5,0.5);
		glutSolidTorus(5, 10, 10, 6);
	glPopMatrix();
}

void desenhaCarrossel(float x, float y, float z, float angulo){
	glColor3f(0.7,0.7,0.7);
	glPushMatrix();
		glTranslatef(x,y,z);
		glutSolidCube(15);
		glRotatef(angulo, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		glColor3f(0.5,0.5,0.5);
		glutSolidTorus(5, 15, 10, 8);
	glPopMatrix();
}

void desenhaXicara(float x, float y, float z, float tamanho){
	glColor3f(0.8,0.4,0.6);
	glPushMatrix();
		glTranslatef(x,y+tamanho,z);
		glutWireTeapot(tamanho);
	glPopMatrix();
}

void desenhaQueda(float x, float y, float z, int posicao){
	glColor3f(0.7,0.7,0.7);
	glPushMatrix();
		glTranslatef(x,y,z);
		glPushMatrix();
			glScalef(1,20,1);
			glutSolidCube(5);
		glPopMatrix();
		glColor3f(0.5,0.5,0.5);
		glTranslatef(0,posicao,0);
		glutSolidCube(8);
	glPopMatrix();
}

//função que desenhará tudo o que aparece na tela
void desenhaCena() {
		float corluz[4]={0.5,0.5,0.5,1};

    //esfera de raio 100
    camera.x = 100 * sin(phi) * cos(teta);  //coordenada x denotada em coordenadas esféricas
    camera.z = 100 * sin(phi) * sin(teta); //coordenada z denotada em coordenadas esféricas
    camera.y = 100 * cos(phi);          //coordenada y denotada em coordenadas esféricas

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // carrega a matriz identidade do modelo de visualização,
    // sempre utilize antes de usar LookAt
    glLoadIdentity();

    //define um LookAt diferente para cada modo da câmera, veja abaixo o
    // uso de cada um
    switch (modoCAM) {
	    case BRINQUEDOS:
				switch(foco){
					case RODA:
						gluLookAt(0, 20, 50,
											0, 0, 0,
											0, 1, 0);
						break;
					case CARROSSEL:
						gluLookAt(-40, 20, 50,
											-40, 0, 20,
											0, 1, 0);
						break;
					case QUEDA:
						gluLookAt(60, 20, -20,
											60, 20, 50,
											0, 1, 0);
						break;
					case XICARA:
						gluLookAt(50, 10, 10,
											50, 10, -20,
											0, 1, 0);
						break;
				}
				break;
			case PRIMEIRA:
				gluLookAt(xCursor, 20, zCursor,
						xCursor+camera.x, camera.y+20, zCursor+camera.z,
						0, 1, 0);
				break;
			case GERAL:
	    default:
    		gluLookAt(xCursor, 100, zCursor+100,
                	xCursor+camera.x, camera.y, zCursor+camera.z,
                  0, 1, 0);
				break;
    }

		if(light==1){
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT_MODEL_AMBIENT);
			glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.5);
			glLightfv(GL_LIGHT0, GL_POSITION, corluz);
			glLightfv(GL_LIGHT0, GL_AMBIENT, corluz);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, corluz);
			glLightfv(GL_LIGHT0, GL_SPECULAR, corluz);
		}
		else{
			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT_MODEL_AMBIENT);
		}
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, corluz);

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, corluz);
		desenhaChao();
		desenhaFloresta();
		desenhaRoda(anguloRoda);
		desenhaXicara(50,0,-20, 10);
		desenhaCarrossel(-40,0,20, anguloRoda);
		desenhaQueda(60, 0, 50, posicaoQueda);
    glutSwapBuffers();
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(1, 1);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition (0, 0);

    glutCreateWindow("Playground");

    glutDisplayFunc(desenhaCena);
    glutReshapeFunc(redimensiona);
    // atualização próxima de 60fps (1000/16 = 62.5 fps
    glutTimerFunc(16, atualiza, 16);

    glutSpecialFunc(special);
		glutKeyboardFunc(teclado);
    // usada para capturar o posicionamento do mouse
    glutPassiveMotionFunc(posicionaCamera);

    inicializa();
    glutMainLoop();

    return 0;
}
