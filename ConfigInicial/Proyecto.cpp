/*
* Proyecto final de computación gráfica (teoría)
* Entrega el 14/05/2025
* Integrantes:
* - Alejandro 422066992
* - Alberto 313113439
* - Fernando 318273745
*/
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "stb_image.h"
#include "Texture.h"

#include <Windows.h>
#include <mmsystem.h>
#include <iostream>

#pragma comment(lib, "winmm.lib")

const GLuint WIDTH = 1200, HEIGHT = 900;
int SCREEN_WIDTH, SCREEN_HEIGHT;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

Camera camera(glm::vec3(0.0f, 15.0f, 3.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool showComputer = false; // Variable para controlar la visibilidad

// Variables para la animación
float globalAnimationTime = -1.0f;
bool animationPlaying = false;
float animationSpeed = 2.0f;

// Variables para la animación de restauración gradual
bool restoringWalls = false;  // Bandera para activar la restauración
float wallRestoreProgress = 0.0f;  // Progreso de la restauración (0.0 = completamente reducido)
float wallRestoreStep = 0.02f;  // Paso de restauración por iteración
float wallRestoreTimer = 0.0f;  // Temporizador para controlar el tiempo


// Variables para la animación de reducción gradual
bool shrinkingWalls = false;  // Bandera para activar la animación
float wallShrinkProgress = 1.0f;  // Progreso de la escala (1.0 = tamaño completo)
float wallShrinkStep = 0.02f;  // Paso de reducción por iteración
float wallShrinkDelay = 0.05f;  // Tiempo entre pasos (segundos)
float wallShrinkTimer = 0.0f;  // Temporizador para controlar el tiempo

bool change = false;
float rotBall = 0;
float transBall = 0; //Valor inicial de transformacion de la pelota
bool AnimBall = false; //Bandera para indicar si la animacion esta activa
bool destroywalls = false; //Bandera para indicar si se destruyen las paredes
bool reappearwalls = false; //Bandera para indicar si se vuelven a aparecer las paredes
bool arriba = true; //Bandera para indicar la direccion
bool rayo = false; //Bandera para indicar si el rayo se activa

bool shrinkCPUs = false;
float shrinkProgress = 1.0f;  // Inicia en 1.0 (tamaño normal)
float shrinkStep = 0.2f;      // Paso de reducción (0.2 unidades)
float shrinkDelay = 0.5f;     // Tiempo entre pasos (segundos)
float shrinkTimer = 0.0f;     // Temporizador
bool CPUsVanished = false;

bool encendidas = false; //Bandera para luces

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.4f, 20.0f, -15.5f),
    glm::vec3(0.4f,20.0f,  10.5f),
    glm::vec3(0.4f,20.0f, -40.5f),
    glm::vec3(0.0f,0.0f, 0.0f)
};

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
glm::vec3 Light1 = glm::vec3(0);

// Estructura para keyframes de animación
struct Keyframe {
    float time;
    glm::vec3 position;
    float rotation;
    glm::vec3 scale;
};

// Estructura para componentes de la computadora
struct ComputerComponent {
    Model* model;
    std::vector<Keyframe> keyframes;
    bool isAnimating;
    bool hasAnimated;
    float animationStartTime;
    float animationDuration;
};

struct ComputerInstance {
    glm::vec3 position;
    float     rotationY;
    glm::vec3 scale;
};

std::vector<ComputerComponent> components;

// Función para interpolar entre keyframes
Keyframe InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t) {
    Keyframe result;
    result.time = glm::mix(a.time, b.time, t);
    result.position = glm::mix(a.position, b.position, t);
    result.rotation = glm::mix(a.rotation, b.rotation, t);
    result.scale = glm::mix(a.scale, b.scale, t);
    return result;
}

// Función para obtener el keyframe actual
Keyframe GetCurrentKeyframe(const ComputerComponent& component, float currentTime) {
    if (component.keyframes.empty())
        return Keyframe{ 0, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f) };

    float animTime = currentTime - component.animationStartTime;
    if (animTime <= component.keyframes.front().time)
        return component.keyframes.front();
    if (animTime >= component.keyframes.back().time)
        return component.keyframes.back();

    for (size_t i = 0; i + 1 < component.keyframes.size(); ++i) {
        if (animTime >= component.keyframes[i].time &&
            animTime < component.keyframes[i + 1].time)
        {
            float t = (animTime - component.keyframes[i].time) /
                (component.keyframes[i + 1].time - component.keyframes[i].time);
            return InterpolateKeyframes(component.keyframes[i], component.keyframes[i + 1], t);
        }
    }
    return component.keyframes.back();
}

/*********************/
bool animBoy = false;
glm::vec3 boyPos(-25.768f, 8.2f, 60.331f); // posición inicial del boy  

// Partes del cuerpo
float cuerpoBoy = 0.0f;
float pantIzq = 0.0f;
float pantDer = 0.0f;
float piernaIzq = 0.0f;
float piernaDer = 0.0f;
float brazoDer = 0.0f;
float brazoIzq = 0.0f;
float bicepDer = 0.0f;
float bicepIzq = 0.0f;

bool stepBoy = false;
float rotBoy = 180.0f;

// Variables para KeyFrames
float boyPosX = -25.768f;
float boyPosY = 8.2f;
float boyPosZ = 60.331f;

#define MAX_FRAMES_BOY 400
int i_max_steps = 5;
int i_curr_steps_boy = 0;

typedef struct _frameBoy {
    float incX, incY, incZ;
    float boyPosX, boyPosY, boyPosZ;
    float sktPosX, sktPosY, sktPosZ;
    float rotBoy, rotBoyInc;
    float cuerpoBoy, cuerpoBoyInc;
    float pantIzq, pantIzqInc;
    float pantDer, pantDerInc;
    float piernaDer, piernaDerInc;
    float piernaIzq, piernaIzqInc;
    float brazoDer, brazoDerInc;
    float brazoIzq, brazoIzqInc;
    float bicepDer, bicepDerInc;
    float bicepIzq, bicepIzqInc;
} FRAME_BOY;

FRAME_BOY KeyFrameBoy[MAX_FRAMES_BOY];
int FrameIndexBoy = 0;
int playIndexBoy = 0;
bool playBoy = false;

void saveFrameBoy() {
    if (FrameIndexBoy >= MAX_FRAMES_BOY) return;

    KeyFrameBoy[FrameIndexBoy].boyPosX = boyPosX;
    KeyFrameBoy[FrameIndexBoy].boyPosY = boyPosY;
    KeyFrameBoy[FrameIndexBoy].boyPosZ = boyPosZ;

    KeyFrameBoy[FrameIndexBoy].rotBoy = rotBoy;
    KeyFrameBoy[FrameIndexBoy].cuerpoBoy = cuerpoBoy;
    KeyFrameBoy[FrameIndexBoy].pantDer = pantDer;
    KeyFrameBoy[FrameIndexBoy].pantIzq = pantIzq;
    KeyFrameBoy[FrameIndexBoy].piernaDer = piernaDer;
    KeyFrameBoy[FrameIndexBoy].piernaIzq = piernaIzq;
    KeyFrameBoy[FrameIndexBoy].brazoDer = brazoDer;
    KeyFrameBoy[FrameIndexBoy].brazoIzq = brazoIzq;
    KeyFrameBoy[FrameIndexBoy].bicepDer = bicepDer;
    KeyFrameBoy[FrameIndexBoy].bicepIzq = bicepIzq;

    FrameIndexBoy++;
}

void resetElementsBoy() {
    boyPosX = KeyFrameBoy[0].boyPosX;
    boyPosY = KeyFrameBoy[0].boyPosY;
    boyPosZ = KeyFrameBoy[0].boyPosZ;

    rotBoy = KeyFrameBoy[0].rotBoy;
    cuerpoBoy = KeyFrameBoy[0].cuerpoBoy;
    pantDer = KeyFrameBoy[0].pantDer;
    pantIzq = KeyFrameBoy[0].pantIzq;
    piernaIzq = KeyFrameBoy[0].piernaIzq;
    piernaDer = KeyFrameBoy[0].piernaDer;
    brazoIzq = KeyFrameBoy[0].brazoIzq;
    brazoDer = KeyFrameBoy[0].brazoDer;
    bicepIzq = KeyFrameBoy[0].bicepIzq;
    bicepDer = KeyFrameBoy[0].bicepDer;
}

void interpolationBoy() {
    if (playIndexBoy >= FrameIndexBoy - 1) return;

    // Calcular incrementos de posición
    KeyFrameBoy[playIndexBoy].incX = (KeyFrameBoy[playIndexBoy + 1].boyPosX - KeyFrameBoy[playIndexBoy].boyPosX) / i_max_steps;
    KeyFrameBoy[playIndexBoy].incY = (KeyFrameBoy[playIndexBoy + 1].boyPosY - KeyFrameBoy[playIndexBoy].boyPosY) / i_max_steps;
    KeyFrameBoy[playIndexBoy].incZ = (KeyFrameBoy[playIndexBoy + 1].boyPosZ - KeyFrameBoy[playIndexBoy].boyPosZ) / i_max_steps;

    // Calcular incrementos de rotación y partes del cuerpo
    KeyFrameBoy[playIndexBoy].rotBoyInc = (KeyFrameBoy[playIndexBoy + 1].rotBoy - KeyFrameBoy[playIndexBoy].rotBoy) / i_max_steps;
    KeyFrameBoy[playIndexBoy].cuerpoBoyInc = (KeyFrameBoy[playIndexBoy + 1].cuerpoBoy - KeyFrameBoy[playIndexBoy].cuerpoBoy) / i_max_steps;
    KeyFrameBoy[playIndexBoy].pantDerInc = (KeyFrameBoy[playIndexBoy + 1].pantDer - KeyFrameBoy[playIndexBoy].pantDer) / i_max_steps;
    KeyFrameBoy[playIndexBoy].pantIzqInc = (KeyFrameBoy[playIndexBoy + 1].pantIzq - KeyFrameBoy[playIndexBoy].pantIzq) / i_max_steps;
    KeyFrameBoy[playIndexBoy].piernaDerInc = (KeyFrameBoy[playIndexBoy + 1].piernaDer - KeyFrameBoy[playIndexBoy].piernaDer) / i_max_steps;
    KeyFrameBoy[playIndexBoy].piernaIzqInc = (KeyFrameBoy[playIndexBoy + 1].piernaIzq - KeyFrameBoy[playIndexBoy].piernaIzq) / i_max_steps;
    KeyFrameBoy[playIndexBoy].brazoDerInc = (KeyFrameBoy[playIndexBoy + 1].brazoDer - KeyFrameBoy[playIndexBoy].brazoDer) / i_max_steps;
    KeyFrameBoy[playIndexBoy].brazoIzqInc = (KeyFrameBoy[playIndexBoy + 1].brazoIzq - KeyFrameBoy[playIndexBoy].brazoIzq) / i_max_steps;
    KeyFrameBoy[playIndexBoy].bicepDerInc = (KeyFrameBoy[playIndexBoy + 1].bicepDer - KeyFrameBoy[playIndexBoy].bicepDer) / i_max_steps;
    KeyFrameBoy[playIndexBoy].bicepIzqInc = (KeyFrameBoy[playIndexBoy + 1].bicepIzq - KeyFrameBoy[playIndexBoy].bicepIzq) / i_max_steps;
}


void setupBoyWalkAnimation() {
    // Resetear posición y animaciones
    FrameIndexBoy = 0;
    boyPosX = -25.768f;
    boyPosY = 8.2f;
    boyPosZ = 60.331f;

    // Resetear todas las partes del cuerpo
    rotBoy = 180.0f;  // Mirando hacia -Z inicialmente
    cuerpoBoy = 0.0f;
    pantDer = 0.0f;
    pantIzq = 0.0f;
    piernaIzq = 0.0f;
    piernaDer = 0.0f;
    brazoIzq = 0.0f;
    brazoDer = 0.0f;
    bicepIzq = 0.0f;
    bicepDer = 0.0f;

    // 1. Caminar hacia adelante (25 pasos, 50 keyframes)
    for (int i = 0; i < 50; i++) {
        if (i % 2 == 0) boyPosZ -= 1.0f;

        if (i % 2 == 0) {
            piernaDer = -25.0f; piernaIzq = 25.0f;
            pantDer = 25.0f; pantIzq = -25.0f;
            brazoIzq = -30.0f; brazoDer = 30.0f;
        }
        else {
            piernaDer = 25.0f; piernaIzq = -25.0f;
            pantDer = -25.0f; pantIzq = 25.0f;
            brazoIzq = 30.0f; brazoDer = -30.0f;
        }
        saveFrameBoy();
    }

    // 2. Giro a la derecha (90°) con movimiento de pies realista (30 keyframes)
    for (int i = 0; i < 30; i++) {
        rotBoy -= 3.0f; // Giro más suave

        // Movimiento de pies durante el giro
        if (i < 15) {
            // Fase 1: Transferencia de peso al pie izquierdo
            piernaDer = -20.0f - (i * 2.0f); // Pie derecho gira hacia atrás
            piernaIzq = 10.0f + (i * 1.5f);  // Pie izquierdo se levanta
            pantDer = 10.0f + (i * 1.0f);    // Rotación externa
            pantIzq = -5.0f - (i * 0.5f);    // Rotación interna
        }
        else {
            // Fase 2: Transferencia de peso al pie derecho
            piernaDer = -50.0f + ((i - 15) * 3.0f);
            piernaIzq = 32.5f - ((i - 15) * 2.5f);
            pantDer = 25.0f - ((i - 15) * 1.5f);
            pantIzq = -12.5f + ((i - 15) * 1.0f);
        }

        // Brazos balanceándose naturalmente
        float armSwing = sin(i * 0.2f) * 20.0f;
        brazoIzq = -15.0f + armSwing;
        brazoDer = 15.0f - armSwing;

        saveFrameBoy();
    }

    // 3. Caminar hacia la derecha (30 pasos, 60 keyframes)
    for (int i = 0; i < 60; i++) {
        if (i % 2 == 0) boyPosX += 1.0f;

        if (i % 2 == 0) {
            piernaDer = -25.0f; piernaIzq = 25.0f;
            pantDer = 25.0f; pantIzq = -25.0f;
            brazoIzq = -30.0f; brazoDer = 30.0f;
        }
        else {
            piernaDer = 25.0f; piernaIzq = -25.0f;
            pantDer = -25.0f; pantIzq = 25.0f;
            brazoIzq = 30.0f; brazoDer = -30.0f;
        }
        saveFrameBoy();
    }

    // 4. Giro al frente (90°) con movimiento de pies (30 keyframes)
    for (int i = 0; i < 30; i++) {
        rotBoy += 3.0f;

        if (i < 15) {
            piernaIzq = -20.0f - (i * 2.0f);
            piernaDer = 10.0f + (i * 1.5f);
            pantIzq = 10.0f + (i * 1.0f);
            pantDer = -5.0f - (i * 0.5f);
        }
        else {
            piernaIzq = -50.0f + ((i - 15) * 3.0f);
            piernaDer = 32.5f - ((i - 15) * 2.5f);
            pantIzq = 25.0f - ((i - 15) * 1.5f);
            pantDer = -12.5f + ((i - 15) * 1.0f);
        }

        float armSwing = sin(i * 0.2f) * 20.0f;
        brazoDer = -15.0f + armSwing;
        brazoIzq = 15.0f - armSwing;

        saveFrameBoy();
    }

    // 5. Caminar hacia adelante (20 pasos, 40 keyframes)
    for (int i = 0; i < 40; i++) {
        if (i % 2 == 0) boyPosZ -= 1.0f;

        if (i % 2 == 0) {
            piernaDer = -25.0f; piernaIzq = 25.0f;
            pantDer = 25.0f; pantIzq = -25.0f;
            brazoIzq = -30.0f; brazoDer = 30.0f;
        }
        else {
            piernaDer = 25.0f; piernaIzq = -25.0f;
            pantDer = -25.0f; pantIzq = 25.0f;
            brazoIzq = 30.0f; brazoDer = -30.0f;
        }
        saveFrameBoy();
    }

    // 6. Giro de vuelta a la izqueirda (90 grados, 10 keyframes)
    for (int i = 0; i < 10; i++) {
        rotBoy += 9.0f; // Giro gradual de vuelta al frente
        // Posición neutral durante el giro
        piernaDer = 0.0f;
        piernaIzq = 0.0f;
        pantDer = 0.0f;
        pantIzq = 0.0f;
        brazoIzq = 0.0f;
        brazoDer = 0.0f;
        saveFrameBoy();
    }

    // 7. Caminar hacia la izquierda (10 pasos, 20 keyframes)
    for (int i = 0; i < 15; i++) {
        // Avanzar posición en X cada 2 keyframes
        if (i % 2 == 0) {
            boyPosX -= 1.0f;
        }

        // Alternar movimiento de piernas
        if (i % 2 == 0) {
            piernaDer = -25.0f;
            piernaIzq = 25.0f;
            pantDer = 25.0f;
            pantIzq = -25.0f;
            brazoIzq = -30.0f;
            brazoDer = 30.0f;
        }
        else {
            piernaDer = 25.0f;
            piernaIzq = -25.0f;
            pantDer = -25.0f;
            pantIzq = 25.0f;
            brazoIzq = 30.0f;
            brazoDer = -30.0f;
        }
        saveFrameBoy();
    }

    // 8. Giro a la derecha (90°) con movimiento de pies realista (30 keyframes)
    for (int i = 0; i < 30; i++) {
        rotBoy -= 3.0f; // Giro más suave

        // Movimiento de pies durante el giro
        if (i < 15) {
            // Fase 1: Transferencia de peso al pie izquierdo
            piernaDer = -20.0f - (i * 2.0f); // Pie derecho gira hacia atrás
            piernaIzq = 10.0f + (i * 1.5f);  // Pie izquierdo se levanta
            pantDer = 10.0f + (i * 1.0f);    // Rotación externa
            pantIzq = -5.0f - (i * 0.5f);    // Rotación interna
        }
        else {
            // Fase 2: Transferencia de peso al pie derecho
            piernaDer = -50.0f + ((i - 15) * 3.0f);
            piernaIzq = 32.5f - ((i - 15) * 2.5f);
            pantDer = 25.0f - ((i - 15) * 1.5f);
            pantIzq = -12.5f + ((i - 15) * 1.0f);
        }

        // Brazos balanceándose naturalmente
        float armSwing = sin(i * 0.2f) * 20.0f;
        brazoIzq = -15.0f + armSwing;
        brazoDer = 15.0f - armSwing;

        saveFrameBoy();
    }

    // Configurar para reproducir la animación
    resetElementsBoy();
    interpolationBoy();
    playBoy = true;
    playIndexBoy = 0;
    i_curr_steps_boy = 0;
}
/********************/

// Función para actualizar animaciones en secuencia
void UpdateAnimations(float currentTime) {
    if (!animationPlaying) return;

    if (currentTime < 0.0f) { // Inicializar
        globalAnimationTime = 0.0f;
        for (auto& comp : components) {
            comp.isAnimating = false;
            comp.hasAnimated = false;
        }
        if (!components.empty()) {
            components[0].isAnimating = true; // Solo el primer componente inicia
            components[0].animationStartTime = 0.0f;
        }
        return;
    }

    globalAnimationTime += deltaTime * (animationSpeed * 0.5f); // Velocidad reducida al 50%

    // Lógica de secuencia: un componente termina antes que empiece el siguiente
    for (size_t i = 0; i < components.size(); ++i) {
        if (!components[i].hasAnimated) {
            if (i == 0) {
                if (!components[i].isAnimating) {
                    components[i].isAnimating = true;
                    components[i].animationStartTime = globalAnimationTime;
                }
            }
            else if (components[i - 1].hasAnimated && !components[i].isAnimating) {
                components[i].isAnimating = true;
                components[i].animationStartTime = globalAnimationTime;
            }
            break; // Solo un componente se anima a la vez
        }
    }

    // Verificar si todos terminaron
    animationPlaying = std::any_of(components.begin(), components.end(),
        [](const ComputerComponent& c) { return !c.hasAnimated; });
}

std::vector<Keyframe> CreateComponentOrbitKeyframes(
    const glm::vec3& finalPos,  // Posición final original (no se usa directamente)
    const glm::vec3& finalScale, // Escala original (no se modifica)
    float duration)
{
    std::vector<Keyframe> kfs;
    const float orbitRadius = 2.0f; // Radio fijo de 2 unidades

    // Ángulo aleatorio inicial para variar la dirección de órbita
    float randomStartAngle = static_cast<float>(rand() % 360);
    float rotationDirection = (rand() % 2) ? 1.0f : -1.0f; // Sentido horario/antihorario

    // Keyframe 0: Inicia en posición orbital (offset desde el centro)
    glm::vec3 orbitOffset = glm::vec3(
        orbitRadius * sin(glm::radians(randomStartAngle)),
        0.0f,
        orbitRadius * cos(glm::radians(randomStartAngle))
    );
    kfs.push_back({ 0.0f, orbitOffset, 0.0f, glm::vec3(1.0f) }); // Escala 1.0 (neutral)

    // Keyframe 1: Mitad de la órbita (180°)
    float midAngle = randomStartAngle + 180.0f * rotationDirection;
    orbitOffset = glm::vec3(
        orbitRadius * sin(glm::radians(midAngle)),
        0.0f,
        orbitRadius * cos(glm::radians(midAngle))
    );
    kfs.push_back({ duration * 0.5f, orbitOffset, 180.0f * rotationDirection, glm::vec3(1.0f) });

    // Keyframe 2: Completa la órbita (360°) y vuelve al centro
    kfs.push_back({ duration * 0.8f, orbitOffset * 0.3f, 270.0f * rotationDirection, glm::vec3(1.0f) }); // Suavizado
    kfs.push_back({ duration, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f) }); // Centro exacto

    return kfs;
}

void RenderComponent(Shader& shader,
    ComputerComponent& component,
    float currentTime,
    const glm::mat4& parentTransform)
{
    if (!component.isAnimating && !component.hasAnimated) return;

    Keyframe cf = GetCurrentKeyframe(component, currentTime);

    // 1. Matriz base (transformación padre: posición/escala/rotación original)
    glm::mat4 modelMatrix = parentTransform;

    // 2. Aplicar offset orbital (traslación relativa)
    modelMatrix = glm::translate(modelMatrix, cf.position);

    // 3. Rotación durante la órbita (sobre su propio eje)
    modelMatrix = glm::rotate(modelMatrix, glm::radians(cf.rotation), glm::vec3(0.0f, 1.0f, 0.0f));

    // Enviar matriz al shader
    glUniformMatrix4fv(
        glGetUniformLocation(shader.Program, "model"),
        1, GL_FALSE, glm::value_ptr(modelMatrix)
    );

    // Efecto visual (opcional: resaltar componente activo)
    if (component.isAnimating) {
        float progress = (currentTime - component.animationStartTime) / component.animationDuration;
        glUniform3f(
            glGetUniformLocation(shader.Program, "material.emissive"),
            progress, progress * 0.5f, 0.0f // Brillo anaranjado
        );
    }

    component.model->Draw(shader);

    // Restaurar valores
    if (component.isAnimating) {
        glUniform3f(glGetUniformLocation(shader.Program, "material.emissive"), 0.0f, 0.0f, 0.0f);
    }

    // Actualizar estado al finalizar
    if (component.isAnimating && (currentTime - component.animationStartTime) >= component.animationDuration) {
        component.isAnimating = false;
        component.hasAnimated = true;
    }
}

void RenderComputer(Shader& shader,
    float currentTime,
    const glm::mat4& parentTransform)
{
    if (!showComputer && !animationPlaying) return; // No renderizar si no se debe mostrar

    for (auto& comp : components) {
        RenderComponent(shader, comp, currentTime, parentTransform);
    }
}

// Estructuras para el resto de la escena
struct ModelInstance {
    glm::vec3 position;
    float rotationY;
    glm::vec3 scale;
};

struct Workstation {
    ModelInstance desk;
    ModelInstance cpu1;
    ModelInstance cpu2;
    ModelInstance chair1;
    ModelInstance chair2;
};

void RenderInstance(Shader& shader, Model& model, const ModelInstance& ins, bool isCPU = false) {
    glm::mat4 M(1.0f);
    M = glm::rotate(M, glm::radians(ins.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    M = glm::translate(M, ins.position);

    if (isCPU) {
        if (CPUsVanished) {
            M = glm::scale(M, glm::vec3(0.0f));  // Escala 0 (invisible)
        }
        else {
            M = glm::scale(M, ins.scale * shrinkProgress);  // Escala actual (1.0, 0.8, 0.6...)
        }
    }
    else {
        M = glm::scale(M, ins.scale);  // Objetos no-CPU
    }

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(M));
    model.Draw(shader);
}

int main() {

    //Musica de fondo
    std::cout << "Playing music \n";
    PlaySound(TEXT("Debug/blueblood.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto final Equipo 6", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader shadowShader("Shader/shadow.vs", "Shader/shadow.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    //Cargar shader para skybox
    Shader skyboxshader("Shader/SkyBox.vs", "Shader/SkyBox.frag");

    // Cargar modelos de la escena
    Model piso((char*)"Models/Proyecto/piso/piso.obj");
    Model pared((char*)"Models/Proyecto/Pared/pared.obj");
    Model techoo((char*)"Models/Proyecto/piso1/piso.obj");
    Model lampara((char*)"Models/Proyecto/Lampara_LEDobj/lamparaLeds.obj");
    Model pizarron((char*)"Models/Proyecto/pizarron/pizarron.obj");
    Model cpu((char*)"Models/Proyecto/computadora/computadora.obj");
    Model silla((char*)"Models/Proyecto/silla/silla.obj");
    Model mesa((char*)"Models/Proyecto/mesa/mesa.obj");
    Model ventanas((char*)"Models/Proyecto/ventana/ventana.obj");
    Model proyector((char*)"Models/Proyecto/ProyectorOBJ/proyector.obj");
    //Nave espacial
    Model Nave((char*)"Models/Proyecto/nave_espacial/nave_espacial.obj");
    Model Rayo((char*)"Models/Proyecto/rayo_laser/Rayo45.obj");

    // Cargar componentes de computadora (animables)
    Model gabinete((char*)"Models/Proyecto/gabinete/gabinete.obj");
    Model placamadre((char*)"Models/Proyecto/placamadre/placamadre.obj");
    Model procesador((char*)"Models/Proyecto/procesador/procesador.obj");
    Model ram((char*)"Models/Proyecto/ram/ram.obj");
    Model ssd((char*)"Models/Proyecto/ssd/ssd.obj");
    Model tarjetagrafica((char*)"Models/Proyecto/tarjetagrafica/tarjetagrafica.obj");
    Model ventilador((char*)"Models/Proyecto/ventilador/ventilador.obj");
    Model ventilador2((char*)"Models/Proyecto/ventilador2/ventilador.obj");
    Model fuente((char*)"Models/Proyecto/fuente/fuente.obj");
    Model monitor((char*)"Models/Proyecto/monitor/monitor.obj");
    Model teclado((char*)"Models/Proyecto/teclado/teclado.obj");

    // Cargar nuevos modelos
    Model mesaasus((char*)"Models/Proyecto/MesaGamerOBJ/MesaASUS.obj");
    Model sillanueva((char*)"Models/Proyecto/sillanueva/sillanueva.obj");
    Model aireact((char*)"Models/Proyecto/AirOld/AireViejo.obj");
    Model airenew((char*)"Models/Proyecto/AirNew/AireNuevo.obj");
    Model pantalla((char*)"Models/Proyecto/Samsung8K/Pantalla.obj");

    // Cargar modelos del estudiante
    Model bicepD((char*)"Models/boy/bicepDER.obj");
    Model bicepI((char*)"Models/boy/bicepIZQ.obj");
    Model brazoD((char*)"Models/boy/brazoDER.obj");
    Model brazoI((char*)"Models/boy/brazoIZQ.obj");
    Model piernaD((char*)"Models/boy/piernaDER.obj");
    Model piernaI((char*)"Models/boy/piernaIZQ.obj");
    Model pantD((char*)"Models/boy/pantDER.obj");
    Model pantI((char*)"Models/boy/pantIZQ.obj");
    Model body((char*)"Models/boy/cuerpo.obj");

    // Configurar la animación de caminar al inicio
    setupBoyWalkAnimation();

    //Arreglos para Skybox
    GLfloat skyboxVertices[] = {
        // Positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };


    GLuint indices[] =
    {  // Note that we start from 0!
        0,1,2,3,
        4,5,6,7,
        8,9,10,11,
        12,13,14,15,
        16,17,18,19,
        20,21,22,23,
        24,25,26,27,
        28,29,30,31,
        32,33,34,35
    };

    // First, set the container's VAO,VBO and EBO (for Skybox))
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Skybox
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    //Load textures
    vector<const GLchar*> faces;
    faces.push_back("Skybox/right.jpg");
    faces.push_back("Skybox/left.jpg");
    faces.push_back("Skybox/top.jpg");
    faces.push_back("Skybox/bottom.jpg");
    faces.push_back("Skybox/back.jpg");
    faces.push_back("Skybox/front.jpg");

    GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

    // Configurar puestos de trabajo
    std::vector<Workstation> workstations = {
        // Fila 1 (izquierda)
        {
            {glm::vec3(-25.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.6f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.6f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.6f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.6f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 2 (izquierda)
        {
            {glm::vec3(-10.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.6f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.6f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-10.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.6f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.6f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 3 (izquierda)
        {
            {glm::vec3(5.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.6f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.6f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(5.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.6f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.6f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 4 (izquierda)
        {
            {glm::vec3(20.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.6f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.6f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(20.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.6f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.6f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 5 (izquierda)
        {
            {glm::vec3(35.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.6f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.6f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(35.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.6f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.6f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 1 (derecha)
        {
            {glm::vec3(-25.0f, 5.7f, 12.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, 9.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.6f, 9.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.6f, 15.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, 25.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.6f, 21.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.6f, 27.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 2 (izquierda)
        {
            {glm::vec3(-10.0f, 5.7f, 12.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, 9.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.6f, 9.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.6f, 15.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-10.0f, 5.7f, 25.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.6f, 21.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.6f, 27.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 3 (izquierda)
        {
            {glm::vec3(5.0f, 5.7f, 12.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, 9.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.6f, 9.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.6f, 15.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(5.0f, 5.7f, 25.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.6f, 21.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.6f, 27.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 4 (izquierda)
        {
            {glm::vec3(20.0f, 5.7f, 12.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, 9.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.6f, 9.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.6f, 15.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(20.0f, 5.7f, 25.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.6f, 21.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.6f, 27.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 5 (izquierda)
        {
            {glm::vec3(35.0f, 5.7f, 12.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, 9.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.6f, 9.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.6f, 15.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(35.0f, 5.7f, 25.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.6f, 21.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.6f, 27.0f), 90.0f, glm::vec3(4.8f)}
        },

    };

    // Configuración de paredes
    std::vector<ModelInstance> walls = {
        {glm::vec3(4.0f, 12.5f, -30.7f), 90.0f, glm::vec3(116.0f, 20.0f, 3.0f)},   //Pared izquierda
        {glm::vec3(55.0f, 12.05f,  34.0f), 90.0f, glm::vec3(15.0f,  20.0f, 7.0f)},  //Pared derecha 1
        {glm::vec3(16.0f, 12.5f,  34.0f), 90.0f, glm::vec3(4.0f,   20.0f, 7.0f)},   //Pared derecha 2
        {glm::vec3(-42.5f,12.5f,  34.0f), 90.0f, glm::vec3(23.0f,  20.0f, 7.0f)},  //Pared derecha 3
        {glm::vec3(8.5f,  7.0f,   36.0f), 90.0f, glm::vec3(80.2f,  9.2f,  3.0f)},  //Pared derecha 4
        {glm::vec3(1.0f,  12.5f, -60.3f),  0.0f, glm::vec3(63.0f,  20.0f,  3.0f)},    //Pared frontal
        {glm::vec3(5.5f,  12.5f,  52.0f),  0.0f, glm::vec3(50.3f,  20.0f,  3.0f)}     //Pared trasera

    };


    std::vector<glm::vec3> originalWallScalesBIG; // Escalas originales de las paredes
    // Configuración de ventanas
    std::vector<ModelInstance> windows = {
        {glm::vec3(33.4f, 16.5f, -33.0f), 0.0f, glm::vec3(20.7f, 10.7f, 30.5f)},
        {glm::vec3(33.4f, 16.5f,  8.5f), 0.0f, glm::vec3(20.7f, 10.7f, 45.7f)}
    };
    std::vector<glm::vec3> originalWindowsScalesBIG; // Escalas originales de las ventanas

    // Configuración de mesas adicionales (solo una definición)
    ModelInstance teacherDesk = { glm::vec3(45.0f, 6.4f, -20.0f), 90.0f, glm::vec3(10.0f,10.0f,18.0f) };
    ModelInstance additionalDesk = { glm::vec3(-40.0f,6.4f, 20.0f),  90.0f, glm::vec3(10.0f,10.0f,18.0f) };

    // Inicializar animaciones de componentes
    components = {
     {&gabinete,       CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.2f},
     {&placamadre,     CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&procesador,     CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&ram,            CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&ssd,            CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&tarjetagrafica, CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&ventilador,     CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.6f},
     {&ventilador2,    CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.6f},
     {&fuente,         CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&monitor,        CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.2f},
     {&teclado,        CreateComponentOrbitKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f}
    };

    // Matrices precalculadas para objetos estáticos
    const glm::mat4 lampTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(0.4f, 20.0f, -15.5f)),
        glm::vec3(34.7f, 12.7f, 10.0f));
    const glm::mat4 lampTransform2 = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(0.4f, 20.0f, 10.5f)),
        glm::vec3(34.7f, 12.7f, 10.0f));
    const glm::mat4 lampTransform3 = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(0.4f, 20.0f, -40.5f)),
        glm::vec3(34.7f, 12.7f, 10.0f));
    const glm::mat4 ceilingTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(2.4f, 23.0f, -4.0f)),
        glm::vec3(68.7f, 5.7f, 114.7f));
    const glm::mat4 floorTransform = glm::scale(glm::translate(
        glm::rotate(glm::mat4(1.0f),
            glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)), //Rotate
        glm::vec3(3.0f, -2.7f, -4.0f)), //Translate
        glm::vec3(60.7f, 90.7f, 115.7f));//145.7f Scale
    const glm::mat4 boardTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(1.0f, 9.0f, -58.0f)),
        glm::vec3(30.0f, 14.0f, 25.0f));

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = static_cast<GLfloat>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (keys[GLFW_KEY_R]) {
            if (!animationPlaying) {
                showComputer = true; // Hacer visible la computadora
                globalAnimationTime = -1.0f;
                animationPlaying = true;
                // Reiniciar estados de componentes
                for (auto& comp : components) {
                    comp.isAnimating = false;
                    comp.hasAnimated = false;
                }
            }
            keys[GLFW_KEY_R] = false;
        }

        std::vector<glm::vec3> originalWallScales;
        std::vector<glm::vec3> originalWindowsScales;


        // Guardar la escala original de las paredes
        for (const auto& wall : walls) {
            originalWallScales.push_back(wall.scale);
        }
        for (const auto& wall : walls) {
            originalWallScalesBIG.push_back(wall.scale);
        }
        for (const auto& wall : windows) {
            originalWindowsScales.push_back(wall.scale);
        }
        for (const auto& wall : windows) {
            originalWindowsScalesBIG.push_back(wall.scale);
        }

        if (shrinkingWalls) {
            wallShrinkTimer += deltaTime;

            if (wallShrinkTimer >= wallShrinkDelay) {
                wallShrinkProgress -= wallShrinkStep;
                wallShrinkTimer = 0.0f;

                // Limitar el progreso a 0
                if (wallShrinkProgress <= 0.0f) {
                    wallShrinkProgress = 0.0f;
                    shrinkingWalls = false;
                    destroywalls = false; // Detener la animación
                }

                // Actualizar la escala de las paredes
                for (size_t i = 0; i < walls.size(); ++i) {
                    walls[i].scale = glm::mix(originalWallScales[i], glm::vec3(0.0f), 1.0f - wallShrinkProgress);
                }
                for (size_t i = 0; i < windows.size(); ++i) {
                    windows[i].scale = glm::mix(originalWindowsScales[i], glm::vec3(0.0f), 1.0f - wallShrinkProgress);
                }
            }
        }

        // Animación de restauración de paredes
        if (restoringWalls) {
            wallRestoreTimer += deltaTime;

            if (wallRestoreTimer >= wallShrinkDelay) {
                wallRestoreProgress += wallRestoreStep;
                wallRestoreTimer = 0.0f;

                // Limitar el progreso a 1.0
                if (wallRestoreProgress >= 1.0f) {
                    wallRestoreProgress = 1.0f;
                    restoringWalls = false;  // Detener la animación
                    reappearwalls = false;
                }

                // Interpolar la escala de las paredes desde 0 hasta la escala original
                for (size_t i = 0; i < walls.size(); ++i) {
                    walls[i].scale = glm::mix(glm::vec3(0.0f), originalWallScalesBIG[i], wallRestoreProgress);
                }
                for (size_t i = 0; i < windows.size(); ++i) {
                    windows[i].scale = glm::mix(glm::vec3(0.0f), originalWindowsScalesBIG[i], wallRestoreProgress);
                }
            }
        }


        // Actualizar animaciones
        UpdateAnimations(globalAnimationTime);

        // Eventos y movimiento de cámara
        glfwPollEvents();
        DoMovement();

        // Clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // Configurar luces
        // Luz direccional
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"),
            -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"),
            0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"),
            0.5f, 0.5f, 0.5f);

        // Configurar luces
        //Load Model



        glUniform1i(glGetUniformLocation(shader.Program, "diffuse"), 0);
        glUniform1i(glGetUniformLocation(shader.Program, "specular"), 1);

        GLint viewPosLoc = glGetUniformLocation(shader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        /*// Luz direccional
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"),
            -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"),
            0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"),
            0.5f, 0.5f, 0.5f);
            */

            // Luz puntual (lámparas)

            // Point light
        glm::vec3 lightColor;
        lightColor.x = abs(sin(glfwGetTime() * Light1.x));
        lightColor.y = abs(sin(glfwGetTime() * Light1.y));
        lightColor.z = sin(glfwGetTime() * Light1.z);

        if (encendidas) {
            // Point light 1
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.2f, 0.2f, 0.2f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 0.4f, 0.4f, 0.4f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 0.5f, 0.5f, 0.5f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f); //Factor de atenuación
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0075f);

            // Point light 2
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].ambient"), 0.2f, 0.2f, 0.2f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].diffuse"), 0.4f, 0.4f, 0.4f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].specular"), 0.5f, 0.5f, 0.5f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].constant"), 1.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].quadratic"), 0.0075f);

            // Point light 3
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].ambient"), 0.7f, 0.7f, 0.8f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].diffuse"), 0.95f, 0.95f, 0.95f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].specular"), 1.0f, 1.0f, 1.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].constant"), 1.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].quadratic"), 0.0075f);
        }
        else {
            // Point light 1
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].ambient"), 0.001f, 0.001f, 0.001f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].diffuse"), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[1].specular"), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].constant"), 1.0f); //Factor de atenuación
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[1].quadratic"), 0.0075f);

            // Point light 2
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].ambient"), 0.001f, 0.001f, 0.001f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].diffuse"), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[2].specular"), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].constant"), 1.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[2].quadratic"), 0.0075f);

            // Point light 3
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].ambient"), 0.001f, 0.001f, 0.001f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].diffuse"), 0.0f, 0.0f, 0.0f);
            glUniform3f(glGetUniformLocation(shader.Program, "pointLights[3].specular"), 0.0f, 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].constant"), 1.0f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].linear"), 0.045f);
            glUniform1f(glGetUniformLocation(shader.Program, "pointLights[3].quadratic"), 0.0075f);

        }

        glm::vec3 lightPos(0.4f, 20.0f, -10.5f);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].position"),
            lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].ambient"),
            0.0f, 0.0f, 0.0f);//Se reduce a 0 la luz extra
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].diffuse"),
            0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].specular"),
            0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].quadratic"), 0.032f);


        // Spotlight (cámara)
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y,
            camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y,
            camera.GetFront().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"),
            0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"),
            1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"),
            glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"),
            glm::cos(glm::radians(15.0f)));

        // Material
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"),
            0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"),
            32.0f);

        // Vista y proyección
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.GetZoom(),
            (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            0.1f, 1000.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"),
            camera.GetPosition().x,
            camera.GetPosition().y,
            camera.GetPosition().z);

        // Get the uniform locations
        shader.Use();

        GLint modelLoc = glGetUniformLocation(shader.Program, "model");
        GLint viewLoc = glGetUniformLocation(shader.Program, "view");
        GLint projLoc = glGetUniformLocation(shader.Program, "projection");

        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


        glm::mat4 nave(1);
        glm::mat4 aire(1);
        glm::mat4 proyectorview(1);

        // Renderizar ventanas
        for (const auto& wi : windows) {
            RenderInstance(shader, ventanas, wi);
        }

        // Dibujar el niño
        glm::mat4 modelBoy(1);
        glm::mat4 modelTemp = modelBoy = glm::translate(modelBoy, glm::vec3(boyPosX, boyPosY, boyPosZ));
        modelTemp = modelBoy = glm::rotate(modelBoy, glm::radians(rotBoy), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTemp = modelBoy = glm::scale(modelBoy, glm::vec3(5.0f, 5.0f, 5.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBoy));
        body.Draw(shader);

        // Bíceps Izquierdo 
        glm::mat4 modelBicepIzq = modelTemp;
        modelBicepIzq = glm::translate(modelBicepIzq, glm::vec3(-12.634f + 12.768f, 1.006f - 0.913f, -8.383f + 8.352f));
        modelBicepIzq = glm::rotate(modelBicepIzq, glm::radians(bicepIzq), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBicepIzq));
        bicepI.Draw(shader);

        // Brazo Izquierdo
        glm::mat4 modelBrazoIzq = modelBicepIzq;
        modelBrazoIzq = glm::translate(modelBrazoIzq, glm::vec3(-12.591f + 12.634, 0.844f - 1.006f, -8.389f + 8.383f));
        modelBrazoIzq = glm::rotate(modelBrazoIzq, glm::radians(brazoIzq), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBrazoIzq));
        brazoI.Draw(shader);

        // Bíceps Derecho
        glm::mat4 modelBicepDer = modelTemp;
        modelBicepDer = glm::translate(modelBicepDer, glm::vec3(-12.88f + 12.768f, 1.006f - 0.913f, -8.383f + 8.352f));
        modelBicepDer = glm::rotate(modelBicepDer, glm::radians(bicepDer), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBicepDer));
        bicepD.Draw(shader);

        // Brazo Derecho
        glm::mat4 modelBrazoDer = modelBicepDer;
        modelBrazoDer = glm::translate(modelBrazoDer, glm::vec3(-12.946f + 12.88, 0.859f - 1.006f, -8.408f + 8.383f));
        modelBrazoDer = glm::rotate(modelBrazoDer, glm::radians(brazoDer), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBrazoDer));
        brazoD.Draw(shader);

        // Pierna Izquierda
        glm::mat4 modelPiernaIzq = modelTemp;
        modelPiernaIzq = glm::translate(modelPiernaIzq, glm::vec3(-12.691f + 12.768f, 0.595f - 0.913f, -8.352f + 8.352f));
        modelPiernaIzq = glm::rotate(modelPiernaIzq, glm::radians(piernaIzq), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPiernaIzq));
        piernaI.Draw(shader);

        // Pantorrilla Izquierda
        glm::mat4 modelPantIzq = modelPiernaIzq;
        modelPantIzq = glm::translate(modelPantIzq, glm::vec3(-12.663f + 12.691f, 0.274f - 0.595f, -8.413f + 8.352f));
        modelPantIzq = glm::rotate(modelPantIzq, glm::radians(pantIzq), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPantIzq));
        pantI.Draw(shader);

        // Pierna Derecha
        glm::mat4 modelPiernaDer = modelTemp;
        modelPiernaDer = glm::translate(modelPiernaDer, glm::vec3(-12.827f + 12.768f, 0.592f - 0.913f, -8.348f + 8.352f));
        modelPiernaDer = glm::rotate(modelPiernaDer, glm::radians(piernaDer), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPiernaDer));
        piernaD.Draw(shader);

        // Pantorrilla Derecha
        glm::mat4 modelPantDer = modelPiernaDer;
        modelPantDer = glm::translate(modelPantDer, glm::vec3(-12.872f + 12.827f, 0.27f - 0.592f, -8.405f + 8.348f));
        modelPantDer = glm::rotate(modelPantDer, glm::radians(pantDer), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPantDer));
        pantD.Draw(shader);


        // Lámpara
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(lampTransform));
        lampara.Draw(shader);

        // Lámpara
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(lampTransform2));
        lampara.Draw(shader);

        // Lámpara
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(lampTransform3));
        lampara.Draw(shader);

        // Techo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(ceilingTransform));
        techoo.Draw(shader);

        // Piso
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(floorTransform));
        piso.Draw(shader);

        // Paredes
        for (const auto& w : walls) {
            RenderInstance(shader, pared, w);
        }


        if (change == false) {
            // Pizarrón
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
                1, GL_FALSE, glm::value_ptr(boardTransform));
            pizarron.Draw(shader);
        }

        // Definición de instancias de computadoras
        std::vector<ComputerInstance> computerInstances = {
            // fila 1 (izquierda):
            { glm::vec3(-24.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },

            // fila 2 (izquierda:
            { glm::vec3(-24.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },

            // fila 3 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },

            // fila 4 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },

            // fila 5 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },

            // fila 1 (derecha):
            { glm::vec3(12.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(18.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(24.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(30.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },

            // fila 2 (derecha):
            { glm::vec3(12.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(18.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(24.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(30.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },

            // fila 3 (derecha):
            { glm::vec3(12.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(18.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(24.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(30.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },

            // fila 4 (derecha):
            { glm::vec3(12.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(18.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(24.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(30.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },

            // fila 5 (derecha):
            { glm::vec3(12.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(18.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(24.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(30.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },

        };

        //Nave
        nave = glm::mat4(1);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(nave));
        glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
        nave = glm::scale(nave, glm::vec3(20.0f, 20.0f, 20.0f));
        nave = glm::translate(nave, glm::vec3(-2.0f, 4.0f, -3.0f));
        nave = glm::translate(nave, glm::vec3(transBall, 0.0f, 0.0f));
        nave = glm::rotate(nave, glm::radians(rotBall), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(nave));
        Nave.Draw(shader);
        //Rayo
        if (rayo == true) {
            //nave = glm::mat4(1);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(nave));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            nave = glm::translate(nave, glm::vec3(0.0f, -1.5f, 1.75f));


            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(nave));
            Rayo.Draw(shader);
        }

        //RenderComputer(shader, globalAnimationTime);
        for (const auto& ci : computerInstances) {
            // 1) monta la matriz padre para ESTA instancia
            glm::mat4 compModel = glm::mat4(1.0f);
            compModel = glm::translate(compModel, ci.position);
            compModel = glm::rotate(compModel,
                glm::radians(ci.rotationY),
                glm::vec3(0.0f, 1.0f, 0.0f));
            compModel = glm::scale(compModel, ci.scale);

            // 2) dibuja todos los componentes bajo esa transformación
            RenderComputer(shader, globalAnimationTime, compModel);
        }


        // Mesas (madera, medio brillo)
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"),
            0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"),
            0.9f, 0.9f, 0.9f);
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"),
            0.2f, 0.2f, 0.2f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"),
            30.0f);



        if (change == false) {
            // Render puestos de trabajo
            for (const auto& ws : workstations) {
                RenderInstance(shader, mesa, ws.desk);
                RenderInstance(shader, silla, ws.chair1);
                RenderInstance(shader, silla, ws.chair2);
            }
            RenderInstance(shader, mesa, teacherDesk);
            RenderInstance(shader, mesa, additionalDesk);

            //AireAcondicionado
            aire = glm::mat4(1);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(aire));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            aire = glm::scale(aire, glm::vec3(15.0f, 5.0f, 5.0f));
            aire = glm::translate(aire, glm::vec3(1.0f, 4.0f, -11.5f));
            aire = glm::rotate(aire, 1.5f, glm::vec3(0.0f, -1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(aire));
            aireact.Draw(shader);

            //Proyector
            proyectorview = glm::mat4(1);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(proyectorview));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            proyectorview = glm::scale(proyectorview, glm::vec3(15.0f, 15.0f, 15.0f));
            proyectorview = glm::translate(proyectorview, glm::vec3(0.0f, 1.2f, -2.0f));
            proyectorview = glm::rotate(proyectorview, 3.0f, glm::vec3(0.0f, -1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(proyectorview));
            proyector.Draw(shader);
        }
        else {
            // Render puestos de trabajo
            for (const auto& ws : workstations) {
                RenderInstance(shader, mesaasus, ws.desk);
                RenderInstance(shader, sillanueva, ws.chair1);
                RenderInstance(shader, sillanueva, ws.chair2);
            }
            RenderInstance(shader, mesaasus, teacherDesk);
            RenderInstance(shader, mesaasus, additionalDesk);

            //AireAcondicionado
            aire = glm::mat4(1);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(aire));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            aire = glm::scale(aire, glm::vec3(15.0f, 5.0f, 5.0f));
            aire = glm::translate(aire, glm::vec3(1.0f, 4.0f, -11.5f));
            aire = glm::rotate(aire, 1.5f, glm::vec3(0.0f, -1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(aire));
            airenew.Draw(shader);

            //Pantalla Samsung
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(boardTransform));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(boardTransform));
            pantalla.Draw(shader);
        }
        // En el bucle de renderizado, modificar la parte donde se renderizan las workstations:
        for (const auto& ws : workstations) {

            RenderInstance(shader, cpu, ws.cpu1, true); // Marcar como CPU
            RenderInstance(shader, cpu, ws.cpu2, true); // Marcar como CPU

        }

        // Also draw the lamp object, again binding the appropriate shader
        lampShader.Use();
        // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
        modelLoc = glGetUniformLocation(lampShader.Program, "model");
        viewLoc = glGetUniformLocation(lampShader.Program, "view");
        projLoc = glGetUniformLocation(lampShader.Program, "projection");

        // Set matrices
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //Fuentes de luz de lamparas
        std::vector<glm::vec3> lampPositions = {
            pointLightPositions[0],  // 
            pointLightPositions[1],  // 
            pointLightPositions[2]   // 
        };

        // Draw each lamp with its own transform
        for (int i = 0; i < 3; i++) {
            glm::mat4 lampModel(1.0f);
            lampModel = glm::translate(lampModel, pointLightPositions[i]);
            lampModel = glm::scale(lampModel, glm::vec3(34.7f, 12.7f, 10.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lampModel));
            lampara.Draw(lampShader);
        }

        glBindVertexArray(0);

        //Draw SkyBox (dentro del gameloop, después de todos los objetos)
        glDepthFunc(GL_LEQUAL); // Función de profundidad para que el SkyBox no interfiera con los objetos.
        skyboxshader.Use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Se quita la traslación porque no queremos que se mueva el SkyBox, debe ser fija.
        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); //Se configura textura en modo cubemap
        glDrawArrays(GL_TRIANGLES, 0, 36); //Se dibuja la caja
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Regresamos a la función de profundidad normal para los objetos.

        glfwSwapBuffers(window);
    }

    //Fuera del game loop se limpian los buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    glfwTerminate();
    return 0;
}

void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)      keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    //Tecla para activar la animación de la nave
    if (keys[GLFW_KEY_N] && action == GLFW_PRESS)
    {
        AnimBall = !AnimBall;

    }
    //Tecla para cambiar por modelos nuevos
    if (keys[GLFW_KEY_R] && action == GLFW_PRESS)
    {
        change = !change;

    }
    if (keys[GLFW_KEY_R] && action == GLFW_PRESS) {
        // Solo activa animación de ensamblaje (componentes)
        if (!animationPlaying) {
            showComputer = true;
            globalAnimationTime = -1.0f;
            animationPlaying = true;
            reappearwalls = true;
            // Reinicia componentes (no CPUs)
            for (auto& comp : components) {
                comp.isAnimating = false;
                comp.hasAnimated = false;
            }
        }
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS || destroywalls == true) {
        if (!shrinkingWalls) {
            shrinkingWalls = true;
            wallShrinkProgress = 1.0f;  // Reiniciar el progreso
            wallShrinkTimer = 0.0f;     // Reiniciar el temporizador
        }
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS || reappearwalls == true) {
        if (!restoringWalls) {
            restoringWalls = true;
            wallRestoreProgress = 0.0f;  // Reiniciar el progreso
            wallRestoreTimer = 0.0f;     // Reiniciar el temporizador
        }
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        setupBoyWalkAnimation();
    }

    //Tecla para activar las luces
    if (keys[GLFW_KEY_L] && action == GLFW_PRESS)
    {
        encendidas = !encendidas;

    }

}

void Animation() {
    // 1. Animación de la nave espacial
    float velocidad = 0.008f;
    float tolerancia = 0.001f;
    float meta = pointLightPositions[0][0];
    float origenModelo = -2.0f;

    if (AnimBall) {
        float posicionActual = origenModelo + transBall;
        rotBall += 0.1f;  // Rotación continua

        // Movimiento hacia la meta
        if (posicionActual < meta - tolerancia) {
            transBall += velocidad;
        }
        else if (posicionActual > meta + tolerancia) {
            transBall -= velocidad;
        }
        else {
            // Nave llegó a su posición final
            transBall = meta - origenModelo;
            AnimBall = false;
            rotBall = 0.0f;
            rayo = true;  // Activa el rayo láser

            // Inicia la reducción escalonada de CPUs
            destroywalls = true;
            if (!shrinkCPUs) {
                shrinkCPUs = true;
                shrinkProgress = 1.0f;
                shrinkTimer = 0.0f;
            }
        }
    }

    // 2. Reducción escalonada de CPUs (solo con rayo activo)
    if (rayo && shrinkCPUs && !CPUsVanished) {
        shrinkTimer += deltaTime;

        // Cada 0.5 segundos reduce el tamaño en 0.2
        if (shrinkTimer >= 0.5f) {
            shrinkProgress -= 0.2f;
            shrinkTimer = 0.0f;

            // Desaparece completamente al llegar a 0
            if (shrinkProgress <= 0.0f) {
                shrinkProgress = 0.0f;
                CPUsVanished = true;
                shrinkCPUs = false;
            }
        }
    }

    // 3. Animación del personaje (boy)
    if (playBoy) {
        if (i_curr_steps_boy >= i_max_steps) {
            playIndexBoy++;
            if (playIndexBoy > FrameIndexBoy - 2) {
                // Si llegamos al final de la animación
                playIndexBoy = 0;
                playBoy = false;
            }
            else {
                i_curr_steps_boy = 0;
                interpolationBoy();
            }
        }
        else {
            // Actualizar posición
            boyPosX += KeyFrameBoy[playIndexBoy].incX;
            boyPosY += KeyFrameBoy[playIndexBoy].incY;
            boyPosZ += KeyFrameBoy[playIndexBoy].incZ;

            // Actualizar rotación y partes del cuerpo
            rotBoy += KeyFrameBoy[playIndexBoy].rotBoyInc;
            cuerpoBoy += KeyFrameBoy[playIndexBoy].cuerpoBoyInc;
            pantDer += KeyFrameBoy[playIndexBoy].pantDerInc;
            pantIzq += KeyFrameBoy[playIndexBoy].pantIzqInc;
            piernaDer += KeyFrameBoy[playIndexBoy].piernaDerInc;
            piernaIzq += KeyFrameBoy[playIndexBoy].piernaIzqInc;
            brazoDer += KeyFrameBoy[playIndexBoy].brazoDerInc;
            brazoIzq += KeyFrameBoy[playIndexBoy].brazoIzqInc;
            bicepDer += KeyFrameBoy[playIndexBoy].bicepDerInc;
            bicepIzq += KeyFrameBoy[playIndexBoy].bicepIzqInc;

            i_curr_steps_boy++;
        }
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = static_cast<GLfloat>(xPos);
        lastY = static_cast<GLfloat>(yPos);
        firstMouse = false;
    }
    GLfloat xOffset = static_cast<GLfloat>(xPos - lastX);
    GLfloat yOffset = static_cast<GLfloat>(lastY - yPos);
    lastX = static_cast<GLfloat>(xPos);
    lastY = static_cast<GLfloat>(yPos);
    camera.ProcessMouseMovement(xOffset, yOffset);
}
