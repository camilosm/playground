#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


enum CAMERAS { GERAL = 1, BRINQUEDOS};
int modoCAM = GERAL;            //variável responsável por guardar o modo de câmera que está sendo utilizado

int xMouse = 0, yMouse = 0;     //variáveis globais que serão usadas na função posicionaCamera
int xCursor, yCursor, zCursor;  //guarda o centro do cursor
float phi = 90, teta = 0;       //ângulos das coordenadas esféricas
float anguloRoda=0;

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

void teclado(unsigned char key, int x, int y) {
    switch (key) {
        case 27:    //aperte ESC para fechar
            exit(0);
            break;
        case 's':   //andar pelo plano X-Z utilizando W A S D
            xCursor++;
            break;
        case 'w':
            xCursor--;
            break;
        case 'a':
            zCursor++;
            break;
        case 'd':
            zCursor--;
            break;
        case '1':
            modoCAM = GERAL;
            break;
        case '2':
            modoCAM = BRINQUEDOS;
            break;
        default:
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
    anguloRoda+=0.1;
		if(anguloRoda>360)
			anguloRoda=0;
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
	// void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
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
	glColor3f(0.5,0.5,0.5);
	glPushMatrix();
		glPushMatrix();
			glutSolidCube(10);
		glPopMatrix();
		glTranslatef(0, 10, 0);
		glRotatef(angulo, 0, 0, 1);
		glColor3f(0,0,0);
		glutWireTorus(5, 10, 10, 10);
	glPopMatrix();
}

//função que desenhará tudo o que aparece na tela
void desenhaCena() {
	int i, j, k;
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
	        gluLookAt(xCursor+0, 20, zCursor+0,
	            xCursor+camera.x, camera.y, zCursor+camera.z,
	            0, 1, 0);
	        break;

			case GERAL:
	    default:
    		gluLookAt(0, 100, 100,   // Z=200
                  0, 0, 0,    // (0, 0, 0) origem do mundo
                  0, 1, 0);  //nesse exemplo mais simples, estamos no ponto X=Y=Z=200 olhando para o ponto 0
				break;
    }

		desenhaChao();
		desenhaFloresta();
		desenhaRoda(anguloRoda);
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

    glutKeyboardFunc(teclado);
    // usada para capturar o posicionamento do mouse
    glutPassiveMotionFunc(posicionaCamera);

    inicializa();
    glutMainLoop();

    return 0;
}
