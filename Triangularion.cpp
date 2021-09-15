

#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <random>
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;

void generatePolygon(float** data, int& size, int &rightMost, float &yMax, float &yMin, float &xMax) {
    data[0][0] = 0.0;
    data[0][1] = 0.0;
    data[1][0] = 1.0;
    data[1][1] = 0.3;
    size = 2;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(20, 25);
    int upper = dis(gen);
    std::uniform_real_distribution<> disx(0.2, 2.0);
    std::uniform_real_distribution<> disy(-1.0, 1.0);
    float startX = 1.0, startY = 1.0;
    int i = 2;
    for (; i < upper + 2; i++) {
        startX += disx(gen);
        startY += disy(gen);
        data[i][0] = startX;
        data[i][1] = startY;
        yMax = startY > yMax ? startY : yMax;
        size++;
    }
    if (startY > 0) {
        startY = 0;
        startX += 1.0;
        data[i][0] = startX;
        data[i][1] = startY;
        i++;
        size++;
    }
    xMax = startX;
    rightMost = i - 1;
    startY = -1.0;
    startX -= 1.0;
    data[i][0] = startX;
    data[i][1] = startY;
    i++;
    size++;
    yMin = -1;
    while (startX > 6) {
        startX -= 3*disx(gen);
        startY -= disy(gen);
        data[i][0] = startX;
        data[i][1] = startY;
        yMin = startY < yMin ? startY : yMin;
        size++;
        i++;
    }
    data[i][0] = 0.0;
    data[i][1] = 0.0;
    size++;
}

//because P1, P2 and P3 are in xy plane, the cross product of vector(P1 P2) and vector(P1 P3) will on z-axis, thus the function only return the z value.
float crossProduct(float* point1, float* point2, float* point3) {
    return (point2[0] - point1[0]) * (point3[1] - point1[1]) - (point2[1] - point1[1]) * (point3[0] - point1[0]);
}

//test if the segment P1-P2 and P3-P4 are intersect
bool isIntersect(float* point1, float* point2, float* point3, float* point4) {
    if (crossProduct(point1, point2, point3) * crossProduct(point1, point2, point4) < 0 &&
        crossProduct(point3, point4, point1) * crossProduct(point3, point4, point2) < 0) {
        return true;
    }
    return false;
}

//for simplicity, assume the input is already x-monotone polygon and vertices are listed in clockwise.And the first input vertex is the left-most vertex.
bool isPolygon(float** data, int size, int rightMost) {
    if (size < 4) return false;
    if (size == 4) return true;
    int upTemp = rightMost - 1;
    for (int down = rightMost+1; down < size; down++) {
        int old = upTemp;
        while (data[upTemp][0] > data[down][0])upTemp--;
        for (int i = old; i >= upTemp; i--) {
            if (isIntersect(data[down], data[down - 1], data[i], data[i + 1]))return false;
        }
    }
    return true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    float** data = new float* [200];
    for (int i = 0; i < 200; i++)data[i] = new float[2];
    int size, rightMost;
    float yMax, yMin, xMax;
    generatePolygon(data, size, rightMost, yMax, yMin, xMax);
    while (!isPolygon(data, size, rightMost)) {
        std::cout << "invalid polygon" << endl;
        generatePolygon(data, size, rightMost, yMax, yMin, xMax);
    }
    for (int i = 0; i < size; i++) {
        std::cout << data[i][0] << " " << data[i][1] << std::endl;
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    float* vertices = new float[2 * size ];
    unsigned int* indices = new unsigned int[size];
    for (int i = 0; i < size; i++) {
        vertices[2 * i] = (1.8 * data[i][0]) / xMax - 0.9;
        vertices[2 * i + 1] = (1.8f * (data[i][1] - yMin)) / (yMax - yMin) - 0.9;
        indices[i] = i;
    }

    unsigned int* triangles = new unsigned int[3*(size-3)];
    float xMin = data[0][0];
    int *stack = new int[size];
    int nt=0, pin = 1, upTemp=1, downTemp=size-2;
    stack[0] = 0;
    bool isUp = true;

    //The x-monotone triangulation algorithm
    for (int i = 0; i < size - 3; i++) {
        if (data[upTemp][0] < data[downTemp][0]) {
            if (pin == 1) {
                stack[pin] = upTemp;
                upTemp++;
                pin++;
                isUp = true;
            }
            else if(isUp){
                if (crossProduct(data[stack[pin - 1]], data[stack[pin - 2]], data[upTemp]) < 0) {
                    stack[pin] = upTemp;
                    upTemp++;
                    pin++;
                }
                else {
                    triangles[3 * nt] = stack[pin - 1];
                    triangles[3 * nt + 1] = stack[pin - 2];
                    triangles[3 * nt + 2] = upTemp;
                    nt++;
                    pin--;
                    i--;
                }
            }
            else {
                for (int j = 0; j < pin-1; j++) {
                    triangles[3 * nt] = stack[j];
                    triangles[3 * nt + 1] = stack[j+1];
                    triangles[3 * nt + 2] = upTemp;
                    nt++;
                }
                stack[0] = stack[pin - 1];
                stack[1] = upTemp;
                pin = 2;
                isUp = true;
                upTemp++;
            }
        }
        else {
            if (pin == 1) {
                stack[pin] = downTemp;
                downTemp--;
                pin++;
                isUp = false;
            }
            else if (!isUp) {
                if (crossProduct(data[stack[pin - 1]], data[stack[pin - 2]], data[downTemp]) > 0) {
                    stack[pin] = downTemp;
                    downTemp--;
                    pin++;
                }else{
                    triangles[3 * nt] = stack[pin - 1];
                    triangles[3 * nt + 1] = stack[pin - 2];
                    triangles[3 * nt + 2] = downTemp;
                    nt++;
                    pin--;
                    i--;
                }                
            }
            else {
                for (int j = 0; j < pin - 1; j++) {
                    triangles[3 * nt] = stack[j];
                    triangles[3 * nt + 1] = stack[j + 1];
                    triangles[3 * nt + 2] = downTemp;
                    nt++;
                }
                stack[0] = stack[pin - 1];
                stack[1] = downTemp;
                pin = 2;
                isUp = false;
                downTemp--;
            }
        }
        
    }
    triangles[3 * size - 12] = stack[0];
    triangles[3 * size - 11] = stack[1];
    triangles[3 * size - 10] = rightMost;

    for (int i = 0; i < size - 3; i++) {
        std::cout << triangles[3 * i ] << " " << triangles[3 * i +1] << " " << triangles[3 * i +2] << endl;
    }

    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    const char* fragmentShaderSource = "#version 330 core\n"
        //  "uniform vec4 ourColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0,0.0,0.0,1.0);\n"
        "}\n\0";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    const char* fragmentShaderSource2 = "#version 330 core\n"
        //  "uniform vec4 ourColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0,1.0,1.0,1.0);\n"
        "}\n\0";
    unsigned int fragmentShader2;
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
    glCompileShader(fragmentShader2);

    unsigned int shaderProgram2;
    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int EBO, EBOT;
    glGenBuffers(1, &EBO);

    unsigned int VAO, VAOT;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * size * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size *sizeof(int), indices, GL_STATIC_DRAW);

    //binding the triangle data

    glGenVertexArrays(1, &VAOT);
    glBindVertexArray(VAOT);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBOT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOT);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*(size-3) * sizeof(int), triangles, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int frame = 0;
    int k = 0;

    while (!glfwWindowShouldClose(window))
    {
        Sleep(500);

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAOT);
        glDrawElements(GL_TRIANGLES, 3 * frame, GL_UNSIGNED_INT, 0);
        if (frame < size - 3)frame++;

        glUseProgram(shaderProgram2);
        glBindVertexArray(VAO);
        glDrawElements(GL_LINE_STRIP, size, GL_UNSIGNED_INT, 0);
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    return 0;
}




