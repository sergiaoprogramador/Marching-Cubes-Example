	/*									Exemplo de implementação utilizando o algoritimo de marching cubes
	Uma implementação simples, portátil e completa do algoritimo de visualização de iso-superfície Marching Cubes e Marching Tetrahedrons em um único arquivo de origem
		Site com descrição do algoritimo http://astronomy.swin.edu.au/pbourke/modelling/polygonise///   Este código é de dominío público */

#include <locale>
#include <iostream>
#include <FreeGL\freeglut.h>

//	Este programa requer as bibliotecas OpenGL e GLUT

using namespace std;


struct GLvector	//estrutura de dados 
{
	GLfloat fX;
	GLfloat fY;
	GLfloat fZ;
};
//	Estas tabelas são usadas para que tudo possa ser feito em pequenos loops que você pode ver de uma só vez 

//a2fDeslocamentoVertice Lista as posições, relativas ao vértice 0, para cada um dos 8 vértices do cubo
static const GLfloat a2fDeslocamentoVertice[8][3] =
		// array 8x3, 8 linhas e 3 colunas
{
	{ 0.0, 0.0, 0.0 },
	{ 1.0, 0.0, 0.0 },
	{ 1.0, 1.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 1.0 },
	{ 1.0, 1.0, 1.0 },
	{ 0.0, 1.0, 1.0 }
};

//a2iConexaoBorda Lista o índice dos vértices do ponto final para cada uma das 12 arestas do cubo
static const GLint a2iConexaoBorda[12][2] =
		// array 12x2, 12 linhas e 2 colunas
{
	{ 0,1 },
	{ 1,2 },
	{ 2,3 },
	{ 3,0 },
	{ 4,5 },
	{ 5,6 },
	{ 6,7 },
	{ 7,4 },
	{ 0,4 },
	{ 1,5 },
	{ 2,6 },
	{ 3,7 }
};

//a2fDirecaoBorda Lista a direção do vetor (vértice1-vértice0) para cada aresta do cubo
static const GLfloat a2fDirecaoBorda[12][3] =
		// array 12x3, 12 linhas e 3 colunas
{
	{  1.0,  0.0, 0.0 },
	{  0.0,  1.0, 0.0 },
	{ -1.0,  0.0, 0.0 },
	{  0.0, -1.0, 0.0 },
	{  1.0,  0.0, 0.0 },
	{  0.0,  1.0, 0.0 },
	{ -1.0,  0.0, 0.0 },
	{  0.0, -1.0, 0.0 },
	{  0.0,  0.0, 1.0 },
	{  0.0,  0.0, 1.0 },
	{  0.0,  0.0, 1.0 },
	{  0.0,  0.0, 1.0 }
};

//a2iTetraedroConexaoBorda Lista o índice dos vértices do ponto final para cada um das 6 arestas do teta tetraedro
static const GLint a2iTetraedroConexaoBorda[6][2] =
		// array 6x2, 6 linhas e 2 colunas
{
	{ 0,1 },
	{ 1,2 },
	{ 2,0 },
	{ 0,3 },
	{ 1,3 },
	{ 2,3 } 
};

//a2iTetraedroConexaoBorda Lista o índice dos vértices de um cubo, que compõe cada um dos seis tetraedros dentro do cubo
static const GLint a2iTetraedroEmUmCubo[6][4] =
		// array 6x4, 6 linhas e 4 colunas
{
	{ 0,5,1,6 },
	{ 0,1,2,6 },
	{ 0,2,3,6 },
	{ 0,3,7,6 },
	{ 0,7,4,6 },
	{ 0,4,5,6 },
};

//	variáveis array constante para implementação de cor
static const GLfloat afAmbienteBranco[] = { 0.25, 0.25, 0.25, 1.00 };	
static const GLfloat afAmbienteVermelho[] = { 0.25, 0.00, 0.00, 1.00 };
static const GLfloat afAmbienteVerde[] = { 0.00, 0.25, 0.00, 1.00 };
static const GLfloat afAmbientBlue[] = { 0.00, 0.00, 0.25, 1.00 };
static const GLfloat afBrancoDifuso[] = { 0.75, 0.75, 0.75, 1.00 };
static const GLfloat afVermelhoDifuso[] = { 0.75, 0.00, 0.00, 1.00 };
static const GLfloat afVerdeDifuso[] = { 0.00, 0.75, 0.00, 1.00 };
static const GLfloat afAzulDifuso[] = { 0.00, 0.00, 0.75, 1.00 };
static const GLfloat afBrancoEspecular[] = { 1.00, 1.00, 1.00, 1.00 };
static const GLfloat afVermelhoEspecular[] = { 1.00, 0.25, 0.25, 1.00 };
static const GLfloat afVerdeEspecular[] = { 0.25, 1.00, 0.25, 1.00 };
static const GLfloat afAzulEspecular[] = { 0.25, 0.25, 1.00, 1.00 };


GLenum    eModoPoligono = GL_FILL;
GLint     iTamanhoDados = 16;
GLfloat   fTamanhoPasso = 1.0 / iTamanhoDados;
GLfloat   fValorAlvo = 48.0;
GLfloat   fTempo = 0.0;
GLvector  sPontoOrigem[3];
GLboolean bRotacao = true;
GLboolean bMovimenta = true;
GLboolean bIluminacao = true;


void vIdle();	//define função callback quando não há necessidade de tratar outro evento
void vDesenha();	//define função callback de redesenho da janela GLUT
void vRedimensiona(GLsizei, GLsizei);	//define função para redimensionamento de janela
void vTeclado(unsigned char cTecla, int iX, int iY);	//define função callback para tratamento de eventos no teclado (teclas ASCII)
void vTecladoEspecial(int iTecla, int iX, int iY);	//define função callback para tratamento de teclas especiais (recebe 3 parâmetros do tipo int)

GLvoid vConsoleHelp();
GLvoid vDefineTempo(GLfloat fTempo);
GLfloat fExemplo1(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fExemplo2(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat fExemplo3(GLfloat fX, GLfloat fY, GLfloat fZ);
GLfloat(*fExemplo)(GLfloat fX, GLfloat fY, GLfloat fZ) = fExemplo1;

GLvoid vMarchingCubes();
GLvoid vMarchCube1(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);
GLvoid vMarchCube2(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale);
GLvoid(*vMarchCube)(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale) = vMarchCube1;

void main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	GLfloat afPropriedadesAmbiente[] = { 0.50, 0.50, 0.50, 1.00 };
	GLfloat afPropriedadesDifusas[] = { 0.75, 0.75, 0.75, 1.00 };
	GLfloat afPropriedadesEspecular[] = { 1.00, 1.00, 1.00, 1.00 };

	GLsizei iLargura = 1280.0;
	GLsizei iAltura = 768.0;	

	glutInit(&argc, argv);	//Inicializa a GLUT, recebe os mesmos parâmetros da main
	glutInitWindowPosition(600, 150);	//Indica a posição de localização da janela
	glutInitWindowSize(iLargura, iAltura);	//Específica o tamanho inical em pixels da janela GLUT
	//Define o modo de operação da GLUT.	GLUT_DOUBLE (Define que a GLUT usará dois estados de buffers de cor) buffer visível e buffer oculto.
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);	//GLUT_DEPTH Define que o programa necessita de um buffer de profundidade (z-Buffer) e GLUT_RGB (as cores do programa serão específicada no padrão RGB)
	glutCreateWindow("Visualização 3D - Marching Cubes");	//Cria a janela passando como argumento o título da mesma (Função que cria janela)
	glutDisplayFunc(vDesenha);	//Registra a função callback de redesenho da janela de visualização (Associa a função 'vDesenha' ao evento de Desenho(Callback) da GLUT)
	glutIdleFunc(vIdle);	//função callback quando não há necessidade de tratar outro evento
	glutReshapeFunc(vRedimensiona);	//função responsável por tratrar eventos de redimensionamento da janela GLUT
	glutKeyboardFunc(vTeclado); //Registra a função callback para tratamento das teclas ASCII (Associa a função (vTeclado) ao evento de precionar uma tecla ASCII)
	glutSpecialFunc(vTecladoEspecial);	//função para tratamento de eventos de teclas especiais (que não tem representação na tabela ASCII "F1, F2, PageUp, PageDown")

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, eModoPoligono);

	glLightfv(GL_LIGHT0, GL_AMBIENT, afPropriedadesAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, afPropriedadesDifusas);
	glLightfv(GL_LIGHT0, GL_SPECULAR, afPropriedadesEspecular);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);

	glEnable(GL_LIGHT0);

	glMaterialfv(GL_BACK, GL_AMBIENT, afAmbienteVerde);
	glMaterialfv(GL_BACK, GL_DIFFUSE, afVerdeDifuso);
	glMaterialfv(GL_FRONT, GL_AMBIENT, afAmbientBlue);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, afAzulDifuso);
	glMaterialfv(GL_FRONT, GL_SPECULAR, afBrancoEspecular);
	glMaterialf(GL_FRONT, GL_SHININESS, 25.0);

	vRedimensiona(iLargura, iAltura);	//função redimensionamento janela

	vConsoleHelp();	//exibe texto no console
	glutMainLoop();	//Inicia o processamento e aguarda as interações do usuário (Função que inicia o processamento de eventos da GLUT)
}

GLvoid vConsoleHelp()
{
	cout << "	Exemplo do algoritimo de Marching Cubes" << endl << endl;

	cout << "| + / -             << Aumentar / Diminuir >> a densidade da amostra " << endl;
	cout << "| PageUp / PageDown << Aumentar / Diminuir >> o valor da superfície " << endl;
	cout << "| s                 << Mudar a função de amostra >> " << endl;
	cout << "| c                 << Alternar entre Marching Cubes / Marching Tetrahedrons >> " << endl;
	cout << "| w                 << liga / desliga >> Wireframe (Esqueleto do objeto) " << endl;
	cout << "| l                 << Alternar entre Iluminação / Cor Normal >> " << endl;
	cout << "| Home              << Rotação de Cena ligado / desligado " << endl;
	cout << "| End               << Animação de Ponto de Origem ligado / desligado " << endl;
}


void vRedimensiona(GLsizei iLargura, GLsizei iAltura)	//função para redimensionamento de janela
{
	GLfloat fAspecto, fMetadeTamanho = (1.4142135623730950488016887242097 / 2);

	glViewport(0, 0, iLargura, iAltura);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (iLargura <= iAltura)
	{
		fAspecto = (GLfloat)iAltura / (GLfloat)iLargura;
		glOrtho(-fMetadeTamanho, fMetadeTamanho, -fMetadeTamanho * fAspecto,
			fMetadeTamanho*fAspecto, -10 * fMetadeTamanho, 10 * fMetadeTamanho);
	}
	else
	{
		fAspecto = (GLfloat)iLargura / (GLfloat)iAltura;
		glOrtho(-fMetadeTamanho * fAspecto, fMetadeTamanho*fAspecto, -fMetadeTamanho,
			fMetadeTamanho, -10 * fMetadeTamanho, 10 * fMetadeTamanho);
	}

	glMatrixMode(GL_MODELVIEW);
}

		//	Interação do usuário 
void vTeclado(unsigned char tecla, int iX, int iY)	//função callback para tratamento de eventos no teclado (teclas ASCII)
{
	if (tecla == 27)	//caso tecle ESC o programa encerra
		exit(0);

	if (tecla == 13)	//caso tecle ENTER a janela fica em modo Fullscren
		glutFullScreen();

	if (tecla == 32)	//caso tecle ESPAÇO a janela volta para o modo normal
	{
		glutReshapeWindow(1280.0, 768.0);	//função para remodelar a janela
		glutInitWindowPosition(100, 100);	//função para a posição de localização da janela
	}

	switch (tecla)	//outros tratamento de teclado
	{
	case 119:	//valor decimal para tecla 'w' ASCII
	{
		if (eModoPoligono == GL_LINE)
		{
			eModoPoligono = GL_FILL;
		}
		else
		{
			eModoPoligono = GL_LINE;
		}
		glPolygonMode(GL_FRONT_AND_BACK, eModoPoligono);
	} break;
	case 43:	//valor decimal para tecla '+' ASCII
	case 61:	//valor decimal para tecla '=' ASCII. (fica na mesma posição que a tecla '+')
	{
		++iTamanhoDados;
		fTamanhoPasso = 1.0 / iTamanhoDados;
	} break;
	case 45:	//valor decimal para tecla '-' ASCII
	{
		if (iTamanhoDados > 1)
		{
			--iTamanhoDados;
			fTamanhoPasso = 1.0 / iTamanhoDados;
		}
	} break;
	case 99:	//valor decimal para tecla 'c' ASCII
	{
		if (vMarchCube == vMarchCube1)
		{
			vMarchCube = vMarchCube2;	// Usar o Algoritimo de Marching Tetrahedrons
		}
		else
		{
			vMarchCube = vMarchCube1;	//Usar o algoritimo de Marching Cubes
		}
	} break;
	case 115:	//valor decimal para tecla 's' ASCII
	{
		if (fExemplo == fExemplo1)
		{
			fExemplo = fExemplo2;
		}
		else if (fExemplo == fExemplo2)
		{
			fExemplo = fExemplo3;
		}
		else
		{
			fExemplo = fExemplo1;
		}
	} break;
	case 108:	//valor decimal para tecla 'l' ASCII
	{
		if (bIluminacao)
		{
			glDisable(GL_LIGHTING);	//	use as cores do vertex
		}
		else
		{
			glEnable(GL_LIGHTING);	//	usar a cor do material iluminado
		}

		bIluminacao = !bIluminacao;
	};
	}
}


void vTecladoEspecial(int teclaEspecial, int iX, int iY)	//função callback para tratamento de teclas especiais (recebe 3 parâmetros do tipo int)
{
	switch (teclaEspecial)
	{
	case GLUT_KEY_PAGE_UP:
	{
		if (fValorAlvo < 1000.0)
		{
			fValorAlvo *= 1.1;
		}
	} break;
	case GLUT_KEY_PAGE_DOWN:
	{
		if (fValorAlvo > 1.0)
		{
			fValorAlvo /= 1.1;
		}
	} break;
	case GLUT_KEY_HOME:
	{
		bRotacao = !bRotacao;
	} break;
	case GLUT_KEY_END:
	{
		bMovimenta = !bMovimenta;
	} break;
	}
}



void vIdle()	//função callback quando não há necessidade de tratar outro evento
{
	glutPostRedisplay();
}

void vDesenha()	//função de redesenho da janela glut
{
	static GLfloat fPasso = 0.0;
	static GLfloat fDesvio = 0.0;
	static GLfloat fTempo = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//Pinta a cor de fundo da janela com a cor específicada

	glPushMatrix();

	if (bRotacao)
	{
		fPasso += 4.0;
		fDesvio += 2.5;
	}
	if (bMovimenta)
	{
		fTempo += 0.025;
	}

	vDefineTempo(fTempo);

	glTranslatef(0.0, 0.0, -1.0);
	glRotatef(-fPasso, 1.0, 0.0, 0.0);
	glRotatef(0.0, 0.0, 1.0, 0.0);
	glRotatef(fDesvio, 0.0, 0.0, 1.0);

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	glutWireCube(1.0);
	glPopAttrib();


	glPushMatrix();
	glTranslatef(-0.5, -0.5, -0.5);
	glBegin(GL_TRIANGLES);
	vMarchingCubes();
	glEnd();
	glPopMatrix();


	glPopMatrix();

	glutSwapBuffers(); //função finalização (utiliza dois buffers de cor). Força os comandos para desenho na tela e animação.
}


//fGetOffset Encontra o ponto aproximado de intersecção da superfície entre dois pontos com valores fValor1 e fValor2
GLfloat fGetOffset(GLfloat fValor1, GLfloat fValor2, GLfloat fValorDesejado)
{
	GLdouble fDelta = fValor2 - fValor1;

	if (fDelta == 0.0)
	{
		return 0.5;
	}
	return (fValorDesejado - fValor1) / fDelta;
}


//vGetColor gera uma cor a partir de uma determinada posição e um ponto normal
GLvoid vGetColor(GLvector &rfCor, GLvector &rfPosicao, GLvector &rfNormal)
{
	GLfloat fX = rfNormal.fX;
	GLfloat fY = rfNormal.fY;
	GLfloat fZ = rfNormal.fZ;
	rfCor.fX = (fX > 0.0 ? fX : 0.0) + (fY < 0.0 ? -0.5*fY : 0.0) + (fZ < 0.0 ? -0.5*fZ : 0.0);
	rfCor.fY = (fY > 0.0 ? fY : 0.0) + (fZ < 0.0 ? -0.5*fZ : 0.0) + (fX < 0.0 ? -0.5*fX : 0.0);
	rfCor.fZ = (fZ > 0.0 ? fZ : 0.0) + (fX < 0.0 ? -0.5*fX : 0.0) + (fY < 0.0 ? -0.5*fY : 0.0);
}


GLvoid vNormalizeVector(GLvector &rfResultadoVetor, GLvector &rfVOrigemVetor)
{
	GLfloat fComprimentoAntigo;
	GLfloat fEscalar;

	fComprimentoAntigo = sqrtf((rfVOrigemVetor.fX * rfVOrigemVetor.fX) +
		(rfVOrigemVetor.fY * rfVOrigemVetor.fY) +
		(rfVOrigemVetor.fZ * rfVOrigemVetor.fZ));

	if (fComprimentoAntigo == 0.0)
	{
		rfResultadoVetor.fX = rfVOrigemVetor.fX;
		rfResultadoVetor.fY = rfVOrigemVetor.fY;
		rfResultadoVetor.fZ = rfVOrigemVetor.fZ;
	}
	else
	{
		fEscalar = 1.0 / fComprimentoAntigo;
		rfResultadoVetor.fX = rfVOrigemVetor.fX*fEscalar;
		rfResultadoVetor.fY = rfVOrigemVetor.fY*fEscalar;
		rfResultadoVetor.fZ = rfVOrigemVetor.fZ*fEscalar;
	}
}


/*	Gera um conjunto de dados de amostra fExemplo1(), fExemplo2() e fExemplo3() definem 3 campos escalares 
cujo os valores variam de acordo com as coordenadas de X, Y, Z e pelo valor de fTempo definido por vDefineTempo()	*/
GLvoid vDefineTempo(GLfloat fNovoTempo)
{
	GLfloat fDeslocamento;
	GLint iOrigemNum;

	for (iOrigemNum = 0; iOrigemNum < 3; iOrigemNum++)
	{
		sPontoOrigem[iOrigemNum].fX = 0.5;
		sPontoOrigem[iOrigemNum].fY = 0.5;
		sPontoOrigem[iOrigemNum].fZ = 0.5;
	}

	fTempo = fNovoTempo;
	fDeslocamento = 1.0 + sinf(fTempo);
	sPontoOrigem[0].fX *= fDeslocamento;
	sPontoOrigem[1].fY *= fDeslocamento;
	sPontoOrigem[2].fZ *= fDeslocamento;
}

//fExemplo1 Encontra a distância entre (fX, fY, fZ) três pontos móveis
GLfloat fExemplo1(GLfloat fX, GLfloat fY, GLfloat fZ)
{
	GLdouble fResultado = 0.0;
	GLdouble fDx, fDy, fDz;
	fDx = fX - sPontoOrigem[0].fX;
	fDy = fY - sPontoOrigem[0].fY;
	fDz = fZ - sPontoOrigem[0].fZ;
	fResultado += 0.5 / (fDx*fDx + fDy * fDy + fDz * fDz);

	fDx = fX - sPontoOrigem[1].fX;
	fDy = fY - sPontoOrigem[1].fY;
	fDz = fZ - sPontoOrigem[1].fZ;
	fResultado += 1.0 / (fDx*fDx + fDy * fDy + fDz * fDz);

	fDx = fX - sPontoOrigem[2].fX;
	fDy = fY - sPontoOrigem[2].fY;
	fDz = fZ - sPontoOrigem[2].fZ;
	fResultado += 1.5 / (fDx*fDx + fDy * fDy + fDz * fDz);

	return fResultado;
}

//fExemplo2 encontra a distância de (fX, fY, fZ) de três linhas móveis
GLfloat fExemplo2(GLfloat fX, GLfloat fY, GLfloat fZ)
{
	GLdouble fResultado = 0.0;
	GLdouble fDx, fDy, fDz;
	fDx = fX - sPontoOrigem[0].fX;
	fDy = fY - sPontoOrigem[0].fY;
	fResultado += 0.5 / (fDx*fDx + fDy * fDy);

	fDx = fX - sPontoOrigem[1].fX;
	fDz = fZ - sPontoOrigem[1].fZ;
	fResultado += 0.75 / (fDx*fDx + fDz * fDz);

	fDy = fY - sPontoOrigem[2].fY;
	fDz = fZ - sPontoOrigem[2].fZ;
	fResultado += 1.0 / (fDy*fDy + fDz * fDz);

	return fResultado;
}


//fExemplo2 Define um campo de altura, ligando a distância do centro para as funções seno (sin) e coseno ()
GLfloat fExemplo3(GLfloat fX, GLfloat fY, GLfloat fZ)
{
	GLfloat fAltura = 20.0*(fTempo + sqrt((0.5 - fX)*(0.5 - fX) + (0.5 - fY)*(0.5 - fY)));
	fAltura = 1.5 + 0.1*(sinf(fAltura) + cosf(fAltura));
	GLdouble fResultado = (fAltura - fZ)*50.0;

	return fResultado;
}


	/*vGetNormal() Encontra o gradiente do campo escalar no ponto 
	este gradiente pode ser usado como vértice normal muito preciso para o cálculo de iluminação*/
GLvoid vGetNormal(GLvector &rfNormal, GLfloat fX, GLfloat fY, GLfloat fZ)
{
	rfNormal.fX = fExemplo(fX - 0.01, fY, fZ) - fExemplo(fX + 0.01, fY, fZ);
	rfNormal.fY = fExemplo(fX, fY - 0.01, fZ) - fExemplo(fX, fY + 0.01, fZ);
	rfNormal.fZ = fExemplo(fX, fY, fZ - 0.01) - fExemplo(fX, fY, fZ + 0.01);
	vNormalizeVector(rfNormal, rfNormal);
}


//vMarchCube1 Executa o algoritmo de Marching Cubes em um único cubo
GLvoid vMarchCube1(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale)
{
	extern GLint aiCuboBorda[256];
	extern GLint a2iTabelaConectaTriangulo[256][16];

	GLint iCorner, iVertex, iVertexTest, iEdge, iTriangle, iFlagIndex, iEdgeFlags;
	GLfloat fOffset;
	GLvector sColor;
	GLfloat afCubeValue[8];
	GLvector asEdgeVertex[12];
	GLvector asEdgeNorm[12];

		//	Faz uma cópia local dos valores nos cantos do cubo
	for (iVertex = 0; iVertex < 8; iVertex++)
	{
		afCubeValue[iVertex] = fExemplo(fX + a2fDeslocamentoVertice[iVertex][0] * fScale,
			fY + a2fDeslocamentoVertice[iVertex][1] * fScale,
			fZ + a2fDeslocamentoVertice[iVertex][2] * fScale);
	}

		//	Descobre quais vértices estão dentro da superfície e quais estão fora
	iFlagIndex = 0;
	for (iVertexTest = 0; iVertexTest < 8; iVertexTest++)
	{
		if (afCubeValue[iVertexTest] <= fValorAlvo)
			iFlagIndex |= 1 << iVertexTest;
	}

		//	Descobre quais arestas são interceptadas pela superfície
	iEdgeFlags = aiCuboBorda[iFlagIndex];

		//	Se o cubo estiver totalmente dentro ou fora da superfície, então não haverá interseções
	if (iEdgeFlags == 0)
	{
		return;
	}

	/*	Encontra o ponto de intersecção da superfície com cada aresta
		Em seguida, encontra o valor normal para a superfície nesses pontos	*/
	for (iEdge = 0; iEdge < 12; iEdge++)
	{
		//	Se houver uma intersecção nessa borda 
		if (iEdgeFlags & (1 << iEdge))
		{
			fOffset = fGetOffset(afCubeValue[a2iConexaoBorda[iEdge][0]],
				afCubeValue[a2iConexaoBorda[iEdge][1]], fValorAlvo);

			asEdgeVertex[iEdge].fX = fX + (a2fDeslocamentoVertice[a2iConexaoBorda[iEdge][0]][0] + fOffset * a2fDirecaoBorda[iEdge][0]) * fScale;
			asEdgeVertex[iEdge].fY = fY + (a2fDeslocamentoVertice[a2iConexaoBorda[iEdge][0]][1] + fOffset * a2fDirecaoBorda[iEdge][1]) * fScale;
			asEdgeVertex[iEdge].fZ = fZ + (a2fDeslocamentoVertice[a2iConexaoBorda[iEdge][0]][2] + fOffset * a2fDirecaoBorda[iEdge][2]) * fScale;

			vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].fX, asEdgeVertex[iEdge].fY, asEdgeVertex[iEdge].fZ);
		}
	}


		//	Desenha os triângulos que foi encontrado. Pode haver até cinco por cubo
	for (iTriangle = 0; iTriangle < 5; iTriangle++)
	{
		if (a2iTabelaConectaTriangulo[iFlagIndex][3 * iTriangle] < 0)
			break;

		for (iCorner = 0; iCorner < 3; iCorner++)
		{
			iVertex = a2iTabelaConectaTriangulo[iFlagIndex][3 * iTriangle + iCorner];

			vGetColor(sColor, asEdgeVertex[iVertex], asEdgeNorm[iVertex]);
			glColor3f(sColor.fX, sColor.fY, sColor.fZ);
			glNormal3f(asEdgeNorm[iVertex].fX, asEdgeNorm[iVertex].fY, asEdgeNorm[iVertex].fZ);
			glVertex3f(asEdgeVertex[iVertex].fX, asEdgeVertex[iVertex].fY, asEdgeVertex[iVertex].fZ);
		}
	}
}

	//vMarchTetrahedron Executa o algoritmo de Marching Tetrahedrons em um único Tetraedro
GLvoid vMarchTetrahedron(GLvector *pasTetrahedronPosition, GLfloat *pafTetrahedronValue)
{
	extern GLint aiTetrahedronEdgeFlags[16];
	extern GLint a2iTetrahedronTriangles[16][7];

	GLint iEdge, iVert0, iVert1, iEdgeFlags, iTriangle, iCorner, iVertex, iFlagIndex = 0;
	GLfloat fOffset, fInvOffset, fValue = 0.0;
	GLvector asEdgeVertex[6];
	GLvector asEdgeNorm[6];
	GLvector sColor;

		//	Descobre quais vértices estão dentro da superfície e quais estão fora
	for (iVertex = 0; iVertex < 4; iVertex++)
	{
		if (pafTetrahedronValue[iVertex] <= fValorAlvo)
			iFlagIndex |= 1 << iVertex;
	}

		//	Descobre quais arestas são interceptadas pela superfície
	iEdgeFlags = aiTetrahedronEdgeFlags[iFlagIndex];

	//	Se o Tetraedro estiver totalmente dentro ou fora da superfície, então não haverá interseções 
	if (iEdgeFlags == 0)
	{
		return;
	}
		/*	Encontre o ponto de intersecção da superfície com cada aresta
		então encontre o vetor normal para a superfície nesses pontos	*/
	for (iEdge = 0; iEdge < 6; iEdge++)
	{
		//	Se houver uma intersecção nessa borda
		if (iEdgeFlags & (1 << iEdge))
		{
			iVert0 = a2iTetraedroConexaoBorda[iEdge][0];
			iVert1 = a2iTetraedroConexaoBorda[iEdge][1];
			fOffset = fGetOffset(pafTetrahedronValue[iVert0], pafTetrahedronValue[iVert1], fValorAlvo);
			fInvOffset = 1.0 - fOffset;

			asEdgeVertex[iEdge].fX = fInvOffset * pasTetrahedronPosition[iVert0].fX + fOffset * pasTetrahedronPosition[iVert1].fX;
			asEdgeVertex[iEdge].fY = fInvOffset * pasTetrahedronPosition[iVert0].fY + fOffset * pasTetrahedronPosition[iVert1].fY;
			asEdgeVertex[iEdge].fZ = fInvOffset * pasTetrahedronPosition[iVert0].fZ + fOffset * pasTetrahedronPosition[iVert1].fZ;

			vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].fX, asEdgeVertex[iEdge].fY, asEdgeVertex[iEdge].fZ);
		}
	}
		//	Desenha os triângulos que foi encontrado. Pode haver até 2 por Tetraedro
	for (iTriangle = 0; iTriangle < 2; iTriangle++)
	{
		if (a2iTetrahedronTriangles[iFlagIndex][3 * iTriangle] < 0)
			break;

		for (iCorner = 0; iCorner < 3; iCorner++)
		{
			iVertex = a2iTetrahedronTriangles[iFlagIndex][3 * iTriangle + iCorner];

			vGetColor(sColor, asEdgeVertex[iVertex], asEdgeNorm[iVertex]);
			glColor3f(sColor.fX, sColor.fY, sColor.fZ);
			glNormal3f(asEdgeNorm[iVertex].fX, asEdgeNorm[iVertex].fY, asEdgeNorm[iVertex].fZ);
			glVertex3f(asEdgeVertex[iVertex].fX, asEdgeVertex[iVertex].fY, asEdgeVertex[iVertex].fZ);
		}
	}
}



//vMarchCube2 Executa o algoritmo de Marching Tetrahedrons em um único cubo, fazendo seis chamadas para o vMarchTetrahedron
GLvoid vMarchCube2(GLfloat fX, GLfloat fY, GLfloat fZ, GLfloat fScale)
{
	GLint iVertex, iTetrahedron, iVertexInACube;
	GLvector asCubePosition[8];
	GLfloat  afCubeValue[8];
	GLvector asTetrahedronPosition[4];
	GLfloat  afTetrahedronValue[4];

	//	Faz uma cópia local das posições dos cantos do cubo
	for (iVertex = 0; iVertex < 8; iVertex++)
	{
		asCubePosition[iVertex].fX = fX + a2fDeslocamentoVertice[iVertex][0] * fScale;
		asCubePosition[iVertex].fY = fY + a2fDeslocamentoVertice[iVertex][1] * fScale;
		asCubePosition[iVertex].fZ = fZ + a2fDeslocamentoVertice[iVertex][2] * fScale;
	}
	
	//	Faz uma cópia local dos valores nos cantos do cubo
	for (iVertex = 0; iVertex < 8; iVertex++)
	{
		afCubeValue[iVertex] = fExemplo(asCubePosition[iVertex].fX,
			asCubePosition[iVertex].fY,
			asCubePosition[iVertex].fZ);
	}

	for (iTetrahedron = 0; iTetrahedron < 6; iTetrahedron++)
	{
		for (iVertex = 0; iVertex < 4; iVertex++)
		{
			iVertexInACube = a2iTetraedroEmUmCubo[iTetrahedron][iVertex];
			asTetrahedronPosition[iVertex].fX = asCubePosition[iVertexInACube].fX;
			asTetrahedronPosition[iVertex].fY = asCubePosition[iVertexInACube].fY;
			asTetrahedronPosition[iVertex].fZ = asCubePosition[iVertexInACube].fZ;
			afTetrahedronValue[iVertex] = afCubeValue[iVertexInACube];
		}
		vMarchTetrahedron(asTetrahedronPosition, afTetrahedronValue);
	}
}


//vMarchingCubes Interação sobre todo o conjunto de dados, chamando o vMarchCube em cada cubo
GLvoid vMarchingCubes()
{
	GLint iX, iY, iZ;
	for (iX = 0; iX < iTamanhoDados; iX++)
		for (iY = 0; iY < iTamanhoDados; iY++)
			for (iZ = 0; iZ < iTamanhoDados; iZ++)
			{
				vMarchCube(iX*fTamanhoPasso, iY*fTamanhoPasso, iZ*fTamanhoPasso, fTamanhoPasso);
			}
}


	/*Para qualquer aresta, se um vértice estiver dentro da superfície e o outro estiver fora da superfície, a aresta cruzará a superfície
	Para cada um dos 4 vértices do tetraedro pode haver dois estados possíveis: dentro ou fora da superfície 
	para qualquer tetraedro, existem  2^4 = 16 possíveis conjuntos de estados de vértices
	Esta tabela lista as arestas interceptadas, pela superfície para todos os 16 possíveis estados de vértice
	Existem 6 arestas. Para cada entrada na tabela, se a borda 'n' é cruzada, o bit 'n' é definido como 1*/

GLint aiTetrahedronEdgeFlags[16] =
{
	0x00, 0x0d, 0x13, 0x1e, 0x26, 0x2b, 0x35, 0x38, 0x38, 0x35, 0x2b, 0x26, 0x1e, 0x13, 0x0d, 0x00,
};


	/*Para cada um dos possíveis estados do vértices listados aiTetrahedronEdgeFlags existe uma triangulação específica dos pontos de intersecção das arestas.
	a2iTetrahedronTriangles lista todos eles na forma 0-2 borda tripla com a lista terminando pelo valor inválido -1.
	Tabela criada por Cory Bloyd corysama@yahoo.com*/

GLint a2iTetrahedronTriangles[16][7] =
{
	{ -1, -1, -1, -1, -1, -1, -1 },
{ 0,  3,  2, -1, -1, -1, -1 },
{ 0,  1,  4, -1, -1, -1, -1 },
{ 1,  4,  2,  2,  4,  3, -1 },

{ 1,  2,  5, -1, -1, -1, -1 },
{ 0,  3,  5,  0,  5,  1, -1 },
{ 0,  2,  5,  0,  5,  4, -1 },
{ 5,  4,  3, -1, -1, -1, -1 },

{ 3,  4,  5, -1, -1, -1, -1 },
{ 4,  5,  0,  5,  2,  0, -1 },
{ 1,  5,  0,  5,  3,  0, -1 },
{ 5,  2,  1, -1, -1, -1, -1 },

{ 3,  4,  2,  2,  4,  1, -1 },
{ 4,  1,  0, -1, -1, -1, -1 },
{ 2,  3,  0, -1, -1, -1, -1 },
{ -1, -1, -1, -1, -1, -1, -1 },
};

	/*	Para qualquer aresta, se um vértice estiver dentro da superfície e o outro fora da superfície a aresta cruzará a superfície	
	Para cada um dos 8 vértices do cubo pode haver dois estados possíveis: dentro ou fora da superfície
	Para qualquer cubo são 2^8=256 possíveis conjuntos de estados de vértices
	Esta tabela lista as arestas interceptadas pela superfície pra todos os 256 possíveis estados de vértice
	Existem 12 arestas. Para cada entrada na tabela, se a borda 'n' é cruzada, o bit 'n' é definido como 1 */

GLint aiCuboBorda[256] =
{
	0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
	0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
	0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
	0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
	0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
	0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
	0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
	0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
	0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
	0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
	0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
	0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
	0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
	0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
	0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
	0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
};

	/*	Para cada um dos possíveis estados do vértice listados em aiCuboBorda existe um triângulação específica dos pontos de intersecção das arestas
	a2iTabelaConectaTriangulo lista todos eles na forma de 0-5 de borda tripla com a lista terminando no valor inválido -1
	Por exemplo: a2iTabelaConectaTriangulo[3] lista os 2 triângulos formados quando o canto[0] e o canto[1] estão dentro da superfície, mas o resto do cubo não está.	*/

//  Tabela gerada manualmente em um programa que alguém escreveu a muito tempo

GLint a2iTabelaConectaTriangulo[256][16] =	//	array 256x16, 256 linhas e 16 colunas 
{
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
	{ 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
	{ 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
	{ 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
	{ 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
	{ 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
	{ 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
	{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
	{ 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
	{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
	{ 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
	{ 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
	{ 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
	{ 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
	{ 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
	{ 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
	{ 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
	{ 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
	{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
	{ 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
	{ 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
	{ 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
	{ 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
	{ 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
	{ 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
	{ 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
	{ 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
	{ 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
	{ 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
	{ 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
	{ 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
	{ 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
	{ 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
	{ 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
	{ 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
	{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
	{ 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
	{ 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
	{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
	{ 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
	{ 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
	{ 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
	{ 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
	{ 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
	{ 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
	{ 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
	{ 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
	{ 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
	{ 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
	{ 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
	{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
	{ 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
	{ 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
	{ 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
	{ 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
	{ 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
	{ 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
	{ 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
	{ 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
	{ 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
	{ 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
	{ 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
	{ 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
	{ 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
	{ 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
	{ 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
	{ 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
	{ 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
	{ 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
	{ 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
	{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
	{ 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
	{ 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
	{ 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
	{ 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
	{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
	{ 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
	{ 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
	{ 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
	{ 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
	{ 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
	{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
	{ 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
	{ 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
	{ 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
	{ 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
	{ 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
	{ 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
	{ 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
	{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
	{ 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
	{ 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
	{ 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
	{ 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
	{ 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
	{ 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
	{ 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
	{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
	{ 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
	{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
	{ 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
	{ 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
	{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
	{ 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
	{ 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
	{ 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
	{ 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
	{ 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
	{ 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
	{ 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};
	