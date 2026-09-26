#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <iostream>
#include <cmath> 
#include "data.h"
#include <cstdlib> 
#include <ctime>   
#include <string>  
#include <vector>
#include <map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void display();
void reshape(int w, int h);
void initGL();
void idle();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void drawHUD();
void drawEnvironment();
void drawText(float x, float y, const char* text);

void loadStoreModel();
void drawStoreModel();
void loadGunModel();
void drawGunMesh();
void loadHouseModel();
void drawHouseModel();

void loadBarricadeTexture();
void drawTexturedCube();

ma_engine audioEngine;
ma_sound backgroundMusic;
ma_sound scream;
ma_sound zombie;
bool gameOverSoundPlayed = false;

bool olhandoParaTras = false;
float playerPosX = 0.0f;
float playerPosY = 1.7f;
float playerPosZ = 5.0f;
float lastTime = 0.0f;

float barricadeZPosition = playerPosZ - 3.0f;
float barricadeHealth = 500.0f;
GLuint barricadeTexID = 0;

const int maxAmmo = 30;
int ammo = maxAmmo;
bool gameOver = false;
float gameOverTimer = 0.0f;
std::vector<Zombie> zumbis;
float currentYaw, startYaw, endYaw;
bool isTurning;
float turnTimer;
const int maxZombies = 10;
const float spawnInterval = 1.5f;
const float spawnRange = 6.5f;
const float spawnZpos = -25.0f;

float currentRecoil = 0.0f;
const float recoilForce = 0.4f;
const float recoilRecovery = 2.5f;

float flashTimer = 0.0f;
const float flashDuration = 0.05f;

std::vector<BloodParticle> bloodParticles;

// Dados do modelo da loja
tinyobj::attrib_t store_attrib;
std::vector<tinyobj::shape_t> store_shapes;
std::vector<tinyobj::material_t> store_materials;
std::map<std::string, GLuint> store_textures;

// Dados do modelo da arma
tinyobj::attrib_t gun_attrib;
std::vector<tinyobj::shape_t> gun_shapes;
std::vector<tinyobj::material_t> gun_materials;
GLuint gunTexID = 0;

// Dados do modelo da Casa
tinyobj::attrib_t house_attrib;
std::vector<tinyobj::shape_t> house_shapes;
std::vector<tinyobj::material_t> house_materials;
std::map<std::string, GLuint> house_textures;

// Vari�veis para anima��o de movimento da arma
float gunOffsetY = 0.0f;
const float gunLowerLimit = -2.0f;
const float gunAnimSpeed = 4.0f;


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Zombie Barricade Shooter");

    srand(static_cast<unsigned int>(time(NULL)));

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    initGL();

    ma_result result = ma_engine_init(NULL, &audioEngine);
    if (result != MA_SUCCESS) {
        std::cout << "Falha ao iniciar audio." << std::endl;
        return -1;
    }
    ma_sound_init_from_file(
        &audioEngine,
        "music.mp3",
        MA_SOUND_FLAG_DECODE,
        NULL,
        NULL,
        &backgroundMusic
    );
    ma_sound_init_from_file(
        &audioEngine,
        "scream.mp3",
        MA_SOUND_FLAG_DECODE,
        NULL,
        NULL,
        &scream
    );
    ma_sound_init_from_file(
        &audioEngine,
        "zombie.mp3",
        MA_SOUND_FLAG_DECODE,
        NULL,
        NULL,
        &zombie
    );
    ma_sound_set_looping(&backgroundMusic, MA_TRUE);
    ma_sound_set_looping(&zombie, MA_TRUE);

    ma_sound_set_volume(&backgroundMusic, 0.5f);
    ma_sound_set_volume(&zombie, 0.5f);

    ma_sound_start(&backgroundMusic);
    ma_sound_start(&zombie);


    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    glutMainLoop();

    ma_sound_uninit(&backgroundMusic);
    ma_sound_uninit(&zombie);
    ma_sound_uninit(&scream);
    ma_engine_uninit(&audioEngine);

    return 0;
}
void resetGame() {
    ammo = maxAmmo;
    barricadeHealth = 500.0f;
    zumbis.clear();
    gameOver = false;
    gameOverSoundPlayed = false;
    gameOverTimer = 0.0f;

    currentRecoil = 0.0f;
    playerPosX = 0.0f;
    playerPosY = 1.7f;

    olhandoParaTras = false;
    currentYaw = 0.0f;
    startYaw = 0.0f;
    endYaw = 0.0f;
    isTurning = false;
    ma_sound_stop(&scream);
    ma_sound_start(&backgroundMusic);
    ma_sound_start(&zombie);

    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    std::cout << "Jogo reiniciado!" << std::endl;
}

void loadBarricadeTexture() {
    const char* tex_path = "Modelos/Materiais/tex_di2ng.jpg";

    std::cout << "Carregando textura da Barricada..." << std::endl;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int w, h, c;

    unsigned char* data = stbi_load(tex_path, &w, &h, &c, 4);

    if (data) {
        glGenTextures(1, &barricadeTexID);
        glBindTexture(GL_TEXTURE_2D, barricadeTexID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        std::cout << "Barricada texturizada com sucesso!" << std::endl;
    }
    else {
        std::cerr << "ERRO! Imagem da barricada n�o encontrada em: " << tex_path << std::endl;
    }
}

void drawTexturedCube() {
    if (barricadeTexID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, barricadeTexID);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    // Face da Frente (Z+)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    // Face de Tr�s (Z-)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    // Face de Cima (Y+)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    // Face de Baixo (Y-)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    // Face Direita (X+)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    // Face Esquerda (X-)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// Carregamento do .obj da loja
void loadStoreModel() {
    std::string warn, err;
    const char* obj_filename = "Modelos/loja.obj";
    const char* material_base_dir = "Modelos/";

    std::cout << "Carregando loja..." << std::endl;
    bool ret = tinyobj::LoadObj(&store_attrib, &store_shapes, &store_materials, &warn, &err, obj_filename, material_base_dir, true);

    if (!ret) {
        std::cerr << "Falha ao carregar loja.obj: " << err << std::endl;
        return;
    }

    stbi_set_flip_vertically_on_load(true);

    for (const auto& material : store_materials) {
        if (!material.diffuse_texname.empty()) {
            if (store_textures.find(material.diffuse_texname) == store_textures.end()) {
                std::string tex_path = "Modelos/" + material.diffuse_texname;

                int w, h, c;
                unsigned char* data = stbi_load(tex_path.c_str(), &w, &h, &c, 0);
                if (data) {
                    GLuint tid;
                    glGenTextures(1, &tid);
                    glBindTexture(GL_TEXTURE_2D, tid);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    GLenum fmt = (c == 4) ? GL_RGBA : GL_RGB;
                    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
                    stbi_image_free(data);
                    store_textures[material.diffuse_texname] = tid;
                }
                else {
                    std::cerr << "Erro na textura da loja: " << tex_path << std::endl;
                }
            }
        }
    }
}

// Carregar .obj de arma
void loadGunModel() {
    std::string warn, err;
    const char* obj_filename = "Modelos/arma.obj";
    const char* material_base_dir = "Modelos/";

    std::cout << "Carregando arma..." << std::endl;

    bool ret = tinyobj::LoadObj(&gun_attrib, &gun_shapes, &gun_materials, &warn, &err, obj_filename, material_base_dir, true);

    if (!ret) {
        std::cerr << "Falha ao carregar arma.obj: " << err << std::endl;
        return;
    }

    if (!warn.empty()) {
        std::cout << "Aviso arma.obj: " << warn << std::endl;
    }

    std::cout << "Arma carregada com sucesso. Materiais encontrados: " << gun_materials.size() << std::endl;
}

// Desenhar loja
void drawStoreModel() {
    glEnable(GL_TEXTURE_2D);
    GLfloat mat_white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white);

    for (const auto& shape : store_shapes) {
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            int material_id = shape.mesh.material_ids[f];
            if (material_id != -1 && !store_materials[material_id].diffuse_texname.empty()) {
                glBindTexture(GL_TEXTURE_2D, store_textures[store_materials[material_id].diffuse_texname]);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glBegin(GL_TRIANGLES);
            for (int v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                if (idx.texcoord_index >= 0)
                    glTexCoord2f(store_attrib.texcoords[2 * idx.texcoord_index + 0], store_attrib.texcoords[2 * idx.texcoord_index + 1]);
                if (idx.normal_index >= 0)
                    glNormal3f(store_attrib.normals[3 * idx.normal_index + 0], store_attrib.normals[3 * idx.normal_index + 1], store_attrib.normals[3 * idx.normal_index + 2]);
                glVertex3f(store_attrib.vertices[3 * idx.vertex_index + 0], store_attrib.vertices[3 * idx.vertex_index + 1], store_attrib.vertices[3 * idx.vertex_index + 2]);
            }
            glEnd();
        }
    }
    glDisable(GL_TEXTURE_2D);
}

// Desenho da malha da arma
void drawGunMesh() {
    glDisable(GL_TEXTURE_2D);

    for (const auto& shape : gun_shapes) {
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {

            int material_id = shape.mesh.material_ids[f];

            if (material_id >= 0 && material_id < gun_materials.size()) {
                float r = gun_materials[material_id].diffuse[0];
                float g = gun_materials[material_id].diffuse[1];
                float b = gun_materials[material_id].diffuse[2];

                GLfloat mat_color[] = { r, g, b, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_color);
            }
            else {
                GLfloat default_col[] = { 1.0f, 0.0f, 1.0f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, default_col);
            }

            glBegin(GL_TRIANGLES);
            for (int v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];

                if (idx.normal_index >= 0) {
                    glNormal3f(gun_attrib.normals[3 * idx.normal_index + 0],
                        gun_attrib.normals[3 * idx.normal_index + 1],
                        gun_attrib.normals[3 * idx.normal_index + 2]);
                }

                glVertex3f(gun_attrib.vertices[3 * idx.vertex_index + 0],
                    gun_attrib.vertices[3 * idx.vertex_index + 1],
                    gun_attrib.vertices[3 * idx.vertex_index + 2]);
            }
            glEnd();
        }
    }
    glEnable(GL_TEXTURE_2D);
}

// Carregamento do .obj da Casa
void loadHouseModel() {
    std::string warn, err;
    const char* obj_filename = "Modelos/Casa.obj";
    const char* material_base_dir = "Modelos/";

    std::cout << "Carregando casa..." << std::endl;
    bool ret = tinyobj::LoadObj(&house_attrib, &house_shapes, &house_materials, &warn, &err, obj_filename, material_base_dir, true);

    if (!ret) {
        std::cerr << "Falha ao carregar Casa.obj: " << err << std::endl;
        return;
    }

    // Percorre materiais para carregar texturas
    for (const auto& material : house_materials) {
        if (!material.diffuse_texname.empty()) {
            if (house_textures.find(material.diffuse_texname) == house_textures.end()) {
                std::string tex_path = "Modelos/" + material.diffuse_texname;

                int w, h, c;
                unsigned char* data = stbi_load(tex_path.c_str(), &w, &h, &c, 0);
                if (data) {
                    GLuint tid;
                    glGenTextures(1, &tid);
                    glBindTexture(GL_TEXTURE_2D, tid);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    GLenum fmt = (c == 4) ? GL_RGBA : GL_RGB;
                    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
                    stbi_image_free(data);
                    house_textures[material.diffuse_texname] = tid;
                }
                else {
                    std::cerr << "Erro na textura da casa: " << tex_path << std::endl;
                }
            }
        }
    }
}

// Desenhar Casa
void drawHouseModel() {
    glEnable(GL_TEXTURE_2D);
    GLfloat mat_white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_white);

    for (const auto& shape : house_shapes) {
        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            int material_id = shape.mesh.material_ids[f];
            if (material_id != -1 && !house_materials[material_id].diffuse_texname.empty()) {
                glBindTexture(GL_TEXTURE_2D, house_textures[house_materials[material_id].diffuse_texname]);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glBegin(GL_TRIANGLES);
            for (int v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                if (idx.texcoord_index >= 0)
                    glTexCoord2f(house_attrib.texcoords[2 * idx.texcoord_index + 0], house_attrib.texcoords[2 * idx.texcoord_index + 1]);
                if (idx.normal_index >= 0)
                    glNormal3f(house_attrib.normals[3 * idx.normal_index + 0], house_attrib.normals[3 * idx.normal_index + 1], house_attrib.normals[3 * idx.normal_index + 2]);
                glVertex3f(house_attrib.vertices[3 * idx.vertex_index + 0], house_attrib.vertices[3 * idx.vertex_index + 1], house_attrib.vertices[3 * idx.vertex_index + 2]);
            }
            glEnd();
        }
    }
    glDisable(GL_TEXTURE_2D);
}

void initGL() {
    GLfloat fogColor[] = { 0.05f, 0.05f, 0.1f, 1.0f };
    glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, 0.07f);
    glHint(GL_FOG_HINT, GL_NICEST);

    GLfloat global_ambient[] = { 0.1f, 0.1f, 0.15f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glEnable(GL_LIGHT0);

    GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light_diffuse[] = { 0.4f, 0.4f, 0.5f, 1.0f };
    GLfloat light_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    loadStoreModel();
    loadGunModel();
    loadBarricadeTexture();
    loadHouseModel();

    currentYaw = 0.0f;
    startYaw = 0.0f;
    endYaw = 0.0f;
    isTurning = false;
    turnTimer = 0.0f;

    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void updateParticles(float deltaTime) {
    for (size_t i = 0; i < bloodParticles.size(); ) {
        // F�sica
        bloodParticles[i].x += bloodParticles[i].vx * deltaTime;
        bloodParticles[i].y += bloodParticles[i].vy * deltaTime;
        bloodParticles[i].z += bloodParticles[i].vz * deltaTime;

        // Gravidade
        bloodParticles[i].vy -= 9.8f * deltaTime;

        // Tempo de vida
        bloodParticles[i].life -= deltaTime * 1.0f;

        if (bloodParticles[i].life <= 0.0f) {
            bloodParticles.erase(bloodParticles.begin() + i);
        }
        else {
            ++i;
        }
    }
}

void drawParticles() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);
    for (const auto& p : bloodParticles) {
        glColor3f(p.r, p.g, p.b);

        float s = 0.03f;

        glVertex3f(p.x - s, p.y - s, p.z);
        glVertex3f(p.x + s, p.y - s, p.z);
        glVertex3f(p.x + s, p.y + s, p.z);
        glVertex3f(p.x - s, p.y + s, p.z);

        glVertex3f(p.x - s, p.y - s, p.z);
        glVertex3f(p.x - s, p.y + s, p.z);
        glVertex3f(p.x - s, p.y + s, p.z - (s * 2));
        glVertex3f(p.x - s, p.y - s, p.z - (s * 2));
    }
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Game loop
void idle() {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    updateParticles(deltaTime);

    if (gameOver) {
        gameOverTimer += deltaTime;
        glutPostRedisplay();
        return;
    }

    // Recupera��o do Recoil
    if (currentRecoil > 0.0f) {
        currentRecoil -= recoilRecovery * deltaTime;
        if (currentRecoil < 0.0f) currentRecoil = 0.0f;
    }

    if (flashTimer > 0.0f) {
        flashTimer -= deltaTime;
    }

    // Interpola��o (LERP) para ROTA��O (Yaw)
    if (isTurning) {
        turnTimer += deltaTime;
        float t = turnTimer / turnDuration;

        if (t >= 1.0f) {
            t = 1.0f;
            isTurning = false;
            currentYaw = endYaw;
        }
        else {
            currentYaw = (1.0f - t) * startYaw + t * endYaw;
        }
    }

    // Anima��o da arma
    if (olhandoParaTras) {
        gunOffsetY -= gunAnimSpeed * deltaTime;
        if (gunOffsetY < gunLowerLimit) gunOffsetY = gunLowerLimit;
    }
    else {
        gunOffsetY += gunAnimSpeed * deltaTime;
        if (gunOffsetY > 0.0f) gunOffsetY = 0.0f;
    }

    static float ammoRegenTimer = 0.0f;

    if (olhandoParaTras) {
        ammoRegenTimer += deltaTime;
        if (ammoRegenTimer >= 0.5f) {
            if (ammo < maxAmmo) {
                ammo++;
                ma_engine_play_sound(&audioEngine, "reload.mp3", NULL);
            }
            ammoRegenTimer -= 1.0f;
        }
    }
    else {
        ammoRegenTimer = 0.0f;
    }

    // L�gica de atualiza��o e ataque dos zumbis
    for (auto& zumbi : zumbis) {
        zumbi.update(deltaTime);

        if (!zumbi.isAttacking && zumbi.z >= barricadeZPosition - 1.5) {
            zumbi.isAttacking = true;
            zumbi.attackTimer = 1.0f;
        }

        if (zumbi.isAttacking) {
            zumbi.attackTimer += deltaTime;
            if (zumbi.attackTimer >= 1.0f) {
                barricadeHealth -= 10.0f;
                zumbi.attackTimer -= 1.0f;
                ma_engine_play_sound(&audioEngine, "barricade.mp3", NULL);
                if (barricadeHealth <= 0 && !gameOver) {
                    std::cout << "GAME OVER!" << std::endl;
                    gameOver = true;
                    if (!gameOverSoundPlayed) {
                        ma_sound_start(&scream);
                        ma_sound_stop(&backgroundMusic);
                        ma_sound_stop(&zombie);
                        gameOverSoundPlayed = true;
                    }
                }
            }
        }
    }

    // Zombie Spawn
    static float spawnTimer = 0.0f;
    spawnTimer += deltaTime;

    if (spawnTimer > spawnInterval && zumbis.size() < maxZombies) {
        float randomX = (rand() % 1000 / 1000.0f) * spawnRange - (spawnRange / 2.0f);

        int rng = rand() % 100;
        ZombieType selectedType;

        if (rng < 20) { //20%
            selectedType = RUNNER;
        }
        else if (rng < 50) { //30%
            selectedType = STRAFER;
        }
        else { //50%
            selectedType = NORMAL;
        }
        zumbis.push_back(Zombie(randomX, spawnZpos, selectedType));

        spawnTimer = 0.0f;
    }
    glutPostRedisplay();
}

void drawMuzzleFlash() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPushMatrix();

    glTranslatef(2.0f, 1.3f, -0.1f);

    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    float randomRot = (float)(rand() % 360);
    glRotatef(randomRot, 0.0f, 0.0f, 1.0f);

    float scale = 1.5f;
    glScalef(scale, scale, scale);

    glBegin(GL_TRIANGLES);

    glColor4f(1.0f, 1.0f, 0.0f, 0.9f);

    for (int i = 0; i < 8; i++) {
        float angle = (float)i * (360.0f / 8.0f) * (3.14159f / 180.0f);
        float nextAngle = (float)(i + 1) * (360.0f / 8.0f) * (3.14159f / 180.0f);

        glColor4f(1.0f, 0.2f, 0.0f, 0.0f);
        glVertex3f(cos(angle) * 1.2f, sin(angle) * 1.2f, 0.0f);

        glColor4f(1.0f, 0.2f, 0.0f, 0.0f);
        glVertex3f(cos(nextAngle) * 1.2f, sin(nextAngle) * 1.2f, 0.0f);

        glColor4f(1.0f, 1.0f, 0.0f, 0.9f);
        glVertex3f(0.0f, 0.0f, 0.0f);
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();
}

void drawBox(float width, float height, float depth, float r, float g, float b) {
    glColor3f(r, g, b);
    glPushMatrix();
    glScalef(width, height, depth);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawBlockyZombie(bool isAttacking, float time, ZombieType type) {
    float shirtR, shirtG, shirtB, pantsR, pantsG, pantsB, eyeR, eyeG, eyeB, 
    skinR = 0.26f, skinG = 0.51f, skinB = 0.27f;
    float walkSpeed = 5.0f;
    float attackSpeed = 3.0f;

    switch (type) {
    case RUNNER: // Vermelho/Preto, olho amarelo
        shirtR = 0.9f; shirtG = 0.1f; shirtB = 0.1f; pantsR = 0.1f; pantsG = 0.1f; pantsB = 0.1f;
        eyeR = 1.0f; eyeG = 1.0f; eyeB = 0.0f; walkSpeed = 12.0f; break;
    case STRAFER: // Roxo/Marrom, olho verde
        shirtR = 0.5f; shirtG = 0.0f; shirtB = 0.5f; pantsR = 0.5f; pantsG = 0.25f; pantsB = 0.0f;
        eyeR = 0.0f; eyeG = 1.0f; eyeB = 0.0f; walkSpeed = 4.0f; break;
    default: // Ciano/Jeans, olho vermelho
        shirtR = 0.0f; shirtG = 0.5f; shirtB = 0.5f; pantsR = 0.15f; pantsG = 0.15f; pantsB = 0.4f;
        eyeR = 1.0f; eyeG = 0.0f; eyeB = 0.0f; break;
    }

    float legAngleLeft = 0.0f;
    float legAngleRight = 0.0f;
    float armAngleLeft = 0.0f;
    float armAngleRight = 0.0f;

    if (isAttacking) {
        float smash = abs(sin(time * attackSpeed)) * 90.0f - 45.0f;
        armAngleLeft = smash;
        armAngleRight = smash;
    }
    else {
        legAngleLeft = sin(time * walkSpeed) * 20.0f;
        legAngleRight = -sin(time * walkSpeed) * 20.0f;

        armAngleLeft = -sin(time * walkSpeed) * 20.0f;
        armAngleRight = sin(time * walkSpeed) * 20.0f;
    }

    // Perna Esquerda
    glPushMatrix();
    glTranslatef(-0.12f, 0.7f, 0.0f);
    glRotatef(legAngleLeft, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.35f, 0.0f);
    drawBox(0.22f, 0.7f, 0.22f, pantsR, pantsG, pantsB);
    glPopMatrix();

    // Perna Direita
    glPushMatrix();
    glTranslatef(0.12f, 0.7f, 0.0f);
    glRotatef(legAngleRight, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.35f, 0.0f);
    drawBox(0.22f, 0.7f, 0.22f, pantsR, pantsG, pantsB);
    glPopMatrix();

    // Tronco
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    drawBox(0.48f, 0.6f, 0.25f, shirtR, shirtG, shirtB);
    glPopMatrix();

    // Cabe�a
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    drawBox(0.4f, 0.4f, 0.4f, skinR, skinG, skinB);

    // Olhos
    glColor3f(eyeR, eyeG, eyeB);
    glPushMatrix();
    glTranslatef(-0.1f, 0.0f, -0.205f);
    glScalef(0.06f, 0.06f, 0.01f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.1f, 0.0f, -0.205f);
    glScalef(0.06f, 0.06f, 0.01f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // Bra�o Esquerdo
    glPushMatrix();
    glTranslatef(-0.36f, 1.25f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(armAngleLeft, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.3f, 0.0f);
    drawBox(0.20f, 0.65f, 0.20f, skinR, skinG, skinB);
    glTranslatef(0.0f, -0.2f, 0.0f);
    drawBox(0.22f, 0.3f, 0.22f, shirtR, shirtG, shirtB);
    glPopMatrix();

    // Bra�o Direito
    glPushMatrix();
    glTranslatef(0.36f, 1.25f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(armAngleRight, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.3f, 0.0f);
    drawBox(0.20f, 0.65f, 0.20f, skinR, skinG, skinB);
    glTranslatef(0.0f, -0.2f, 0.0f);
    drawBox(0.22f, 0.3f, 0.22f, shirtR, shirtG, shirtB);
    glPopMatrix();
}

void spawnHeadExplosion(float x, float y, float z) {
    // Gera 100 peda�os
    for (int i = 0; i < 100; i++) {
        bloodParticles.push_back(BloodParticle(x, y, z));
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lookDistance = 10.0f;
    float rad = currentYaw * M_PI / 180.0f;
    float targetX = playerPosX + lookDistance * sin(rad);
    float targetZ = playerPosZ - lookDistance * cos(rad);

    gluLookAt(playerPosX, playerPosY, playerPosZ, targetX, playerPosY, targetZ, 0.0f, 1.0f, 0.0f);

    GLfloat light_position[] = { 0.0f, 5.0f, playerPosZ - 2.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    drawEnvironment();

    GLfloat mat_barricade[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_barricade);

    glPushMatrix();
    glTranslatef(0.0f, 0.5f, barricadeZPosition);
    glScalef(10.0f, 1.5f, 0.2f);

    drawTexturedCube();

    glPopMatrix();

    // Desenho da loja
    if (olhandoParaTras) {
        // Casa
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, playerPosZ + 8.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glScalef(0.15f, 0.15f, 0.15f);
        drawHouseModel();
        glPopMatrix();
        // Caixas
        glPushMatrix();
        glTranslatef(0.0f, 0.6f, playerPosZ + 3.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glScalef(0.02f, 0.02f, 0.02f);
        drawStoreModel();
        glPopMatrix();
    }

    // Desenho dos zumbis
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    for (auto& zumbi : zumbis) {
        glPushMatrix();

        glTranslatef(zumbi.x, 0.0f, zumbi.z);

        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

        float scale = 1.2f;
        glScalef(scale, scale, scale);

        glDisable(GL_TEXTURE_2D);

        float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        drawBlockyZombie(zumbi.isAttacking, time, zumbi.type);

        glEnable(GL_TEXTURE_2D);
        glPopMatrix();

        // Hitbox (Debug)
        /*
        glPushMatrix();
        float zScale = 1.5f;
        float headCenterY = 1.2f * zScale;
        float headSize = 0.35f * zScale;
        glTranslatef(zumbi.x, headCenterY, zumbi.z);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);
        glScalef(headSize, headSize, headSize);
        glutWireCube(1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
        */
    }
    glDisable(GL_COLOR_MATERIAL);

    drawParticles();

    // Desenho da arma
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    // Aplicando o Recoil na posi��o
    glTranslatef(0.7f, -1.0f + gunOffsetY + (currentRecoil * 0.5f), -4.0f + currentRecoil);

    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    // Aplicando Recoil na rota��o
    glRotatef(currentRecoil * 10.0f, 0.0f, 0.0f, 1.0f);

    float gunScale = 0.5f;
    glScalef(gunScale, gunScale, gunScale);

    drawGunMesh();

    if (flashTimer > 0.0f && !olhandoParaTras) {
        drawMuzzleFlash();
    }

    glPopMatrix();

    drawHUD();
    glutSwapBuffers();
}

// Redimensionamento
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int x, int y) {
    if (gameOver) {
        resetGame();
        return;
    }
    switch (key) {
    case 27:
        exit(EXIT_SUCCESS);
        break;

    case 'r':
    case 'R':
        if (!isTurning) {
            isTurning = true;
            turnTimer = 0.0f;
            startYaw = currentYaw;

            if (olhandoParaTras) {
                endYaw = 0.0f;
            }
            else {
                endYaw = 180.0f;
            }
            olhandoParaTras = !olhandoParaTras;
        }
        break;
    }
}

struct Vec3 {
    double x, y, z;
    Vec3 operator-(const Vec3& other) const { return { x - other.x, y - other.y, z - other.z }; }
    Vec3 operator+(const Vec3& other) const { return { x + other.x, y + other.y, z + other.z }; }
    Vec3 operator*(double scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    double dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    double length() const { return sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const { double len = length(); return { x / len, y / len, z / len }; }
};

// Estrutura para definir os limites da caixa
struct AABB {
    Vec3 min;
    Vec3 max;
};

// Algoritmo de interse��o Raio vs AABB (M�todo "Slab")
bool intersectRayAABB(const Vec3& origin, const Vec3& dir, const AABB& box, double& t) {
    double tmin = (box.min.x - origin.x) / dir.x;
    double tmax = (box.max.x - origin.x) / dir.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    double tymin = (box.min.y - origin.y) / dir.y;
    double tymax = (box.max.y - origin.y) / dir.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    double tzmin = (box.min.z - origin.z) / dir.z;
    double tzmax = (box.max.z - origin.z) / dir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    if (tmax < 0) return false;

    t = tmin;
    return true;
}

void mouse(int button, int state, int x, int y) {
    if (gameOver == false) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            if (olhandoParaTras) {
                std::cout << "Clicou na loja! (recarga passiva ativa)" << std::endl;
            }
            else {
                if (ammo <= 0) {
                    std::cout << "Sem municao!" << std::endl;
                    ma_engine_play_sound(&audioEngine, "empty.mp3", NULL);
                    return;
                }
                ammo--;
                std::cout << "Atirou! Municao: " << ammo << std::endl;
                ma_engine_play_sound(&audioEngine, "gunshot.mp3", NULL);
                flashTimer = flashDuration;
                currentRecoil = recoilForce;

                // Raycasting
                int w = glutGet(GLUT_WINDOW_WIDTH);
                int h = glutGet(GLUT_WINDOW_HEIGHT);
                if (h == 0) h = 1;

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                float lookDistance = 10.0f;
                float rad = currentYaw * M_PI / 180.0f;
                float targetX = playerPosX + lookDistance * sin(rad);
                float targetZ = playerPosZ - lookDistance * cos(rad);

                gluLookAt(playerPosX, playerPosY, playerPosZ, targetX, playerPosY, targetZ, 0.0f, 1.0f, 0.0f);

                GLdouble modelview[16];
                GLdouble projection[16];
                GLint viewport[4];

                glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
                glGetDoublev(GL_PROJECTION_MATRIX, projection);
                glGetIntegerv(GL_VIEWPORT, viewport);

                float winY = (float)viewport[3] - (float)y;
                Vec3 nearPoint, farPoint;
                gluUnProject((float)x, winY, 0.0, modelview, projection, viewport, &nearPoint.x, &nearPoint.y, &nearPoint.z);
                gluUnProject((float)x, winY, 1.0, modelview, projection, viewport, &farPoint.x, &farPoint.y, &farPoint.z);

                Vec3 rayOrigin = nearPoint;
                Vec3 rayDir = (farPoint - nearPoint).normalize();

                int hitIndex = -1;
                double closestHitDist = 1000.0;

                float scale = 1.5f;

                float headMinY = 1.025f * scale;
                float headMaxY = 1.375f * scale;
                float headHalfWidth = (0.35f * scale) / 2.0f;

                for (int i = zumbis.size() - 1; i >= 0; i--) {
                    Zombie& z = zumbis[i];

                    AABB box;
                    box.min = { z.x - headHalfWidth, headMinY, z.z - headHalfWidth };
                    box.max = { z.x + headHalfWidth, headMaxY, z.z + headHalfWidth };

                    double t = 0.0;
                    if (intersectRayAABB(rayOrigin, rayDir, box, t)) {
                        if (t < closestHitDist && t > 0) {
                            closestHitDist = t;
                            hitIndex = i;
                        }
                    }
                }

                if (hitIndex != -1) {
                    std::cout << "HEADSHOT!" << std::endl;

                    Zombie& z = zumbis[hitIndex];
                    float headX = z.x;
                    float headY = 1.8f;
                    float headZ = z.z;

                    spawnHeadExplosion(headX, headY, headZ);
                    ma_engine_play_sound(&audioEngine, "explode.mp3", NULL);
                    zumbis.erase(zumbis.begin() + hitIndex);
                }
            }
        }
    }
}

void drawSimpleTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    GLfloat mat_trunk[] = { 0.25f, 0.15f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_trunk);

    glPushMatrix();
    glRotatef(-90, 1.0f, 0.0f, 0.0f);

    GLUquadricObj* quadric = gluNewQuadric();
    gluCylinder(quadric, 0.4, 0.4, 2.5, 8, 1);
    gluDeleteQuadric(quadric);

    glPopMatrix();

    GLfloat mat_leaves[] = { 0.05f, 0.2f, 0.05f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_leaves);

    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    glutSolidCone(1.5, 2.5, 10, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 3.5f, 0.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    glutSolidCone(1.2, 2.0, 10, 2);
    glPopMatrix();

    glPopMatrix();
}

void drawEnvironment() {
    // Ch�o
    glDisable(GL_TEXTURE_2D);
    GLfloat mat_dirt_diffuse[] = { 0.35f, 0.25f, 0.15f, 1.0f };
    GLfloat mat_dirt_ambient[] = { 0.20f, 0.10f, 0.05f, 1.0f };

    GLfloat mat_no_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat mat_no_shininess[] = { 0.0f };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_dirt_diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_dirt_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_no_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_no_shininess);

    float environmentSize = 200.0f;
    float groundLevel = 0.01f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-environmentSize, groundLevel, environmentSize);
    glVertex3f(environmentSize, groundLevel, environmentSize);
    glVertex3f(environmentSize, groundLevel, -environmentSize);
    glVertex3f(-environmentSize, groundLevel, -environmentSize);
    glEnd();

    // Floresta
    for (int i = -12; i <= 12; i++) {
        for (int j = -12; j <= 12; j++) {

            float offsetX = sin(i * 12.3f) * 3.0f;
            float offsetZ = cos(j * 23.4f) * 3.0f;

            float treeX = i * 8.0f + offsetX;
            float treeZ = j * 8.0f + offsetZ + playerPosZ;

            bool shouldDraw = true;

            if (treeZ < playerPosZ + 2.0f) {
                if (treeX > -6.0f && treeX < 6.0f) shouldDraw = false;
            }
            else {
                if (treeX > -7.0f && treeX < 7.0f) shouldDraw = false;
            }

            if (shouldDraw &&
                treeX > -environmentSize && treeX < environmentSize &&
                treeZ > -environmentSize && treeZ < environmentSize) {

                drawSimpleTree(treeX, treeZ);
            }
        }
    }
    glEnable(GL_TEXTURE_2D);
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawHUD() {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);

    if (gameOver) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float alpha = gameOverTimer * 0.8f;
        if (alpha > 0.85f) alpha = 0.85f;

        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(w, 0);
        glVertex2f(w, h);
        glVertex2f(0, h);
        glEnd();

        glDisable(GL_BLEND);

        if (alpha > 0.5f) {
            glColor3f(1.0f, 0.0f, 0.0f);
            const char* msg1 = "GAME OVER";
            drawText(w / 2 - 50, h / 2 + 20, msg1);

            glColor3f(1.0f, 1.0f, 1.0f);
            const char* msg2 = "APERTE QUALQUER TECLA PARA JOGAR NOVAMENTE";
            drawText(w / 2 - 230, h / 2 - 20, msg2);
        }
    }
    else {
        std::string ammoText = "Balas: " + std::to_string(ammo);
        std::string hpText = "Barricada: " + std::to_string((int)barricadeHealth);

        if (olhandoParaTras) {
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        else {
            glColor3f(1.0f, 1.0f, 0.0f);
        }

        float textX = w - 150.0f;
        float textY = h - 30.0f;
        drawText(textX, textY, ammoText.c_str());

        glColor3f(1.0f, 0.2f, 0.2f);
        drawText(20.0f, textY, hpText.c_str());
    }

    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}