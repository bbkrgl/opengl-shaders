#include "EclipseMap.h"

using namespace std;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;

    vertex() {}

    vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : position(position),
                                                                                           normal(normal),
                                                                                           texture(texture) {}
};

struct triangle {
    int vertex1;
    int vertex2;
    int vertex3;

    triangle() {}

    triangle(const int &vertex1, const int &vertex2, const int &vertex3) : vertex1(vertex1), vertex2(vertex2),
                                                                           vertex3(vertex3) {}
};

void EclipseMap::createSphere(int radius, glm::vec3 pos, vector<float>& vertices, vector<int>& indices)
{
    for (int i = 0; i <= verticalSplitCount; i++) {
        float b = M_PI*i/verticalSplitCount;

        for (int j = 0; j <= horizontalSplitCount; j++) {
            float a = 2*M_PI*j/horizontalSplitCount;

            float x = radius*sin(b)*cos(a)+pos.x;
            float y = radius*sin(b)*sin(a)+pos.y;
            float z = radius*cos(b)+pos.z;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            glm::vec3 n = glm::vec3(x,y,z)-pos;
            n = glm::normalize(n);
            vertices.push_back(n.x);
            vertices.push_back(n.y);
            vertices.push_back(n.z);

            vertices.push_back(((float)j)/horizontalSplitCount);
            vertices.push_back(((float)i)/verticalSplitCount);
        }
    }

    for (int i = 0; i < verticalSplitCount; i++) {
        int k1 = i*(horizontalSplitCount+1);
        int k2 = k1+horizontalSplitCount+1;

        for (int j = 0; j < horizontalSplitCount; j++, k1++, k2++) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1+1);
            }

            if (i != (verticalSplitCount-1)) {
                indices.push_back(k1+1);
                indices.push_back(k2);
                indices.push_back(k2+1);
            }
        }
    }
}

void EclipseMap::Render(const char *coloredTexturePath, const char *greyTexturePath, const char *moonTexturePath) {
    // Open window
    GLFWwindow *window = openWindow(windowName, screenWidth, screenHeight);

    // Moon commands
    // Load shaders
    GLuint moonShaderID = initShaders("moonShader.vert", "moonShader.frag");

    glActiveTexture(GL_TEXTURE2);
    initMoonColoredTexture(moonTexturePath, moonShaderID);

    glm::mat4 perspectiveMatrix = glm::perspective(glm::radians(projectionAngle), aspectRatio, near, far);
    glm::mat4 camMatrix = glm::lookAt(cameraPosition, cameraDirection, cameraUp);

    // Set moonVertices
    createSphere(moonRadius, glm::vec3(0,2600,0), moonVertices, moonIndices);

    // Configure Buffers
    GLuint mv_size = moonVertices.size()*sizeof(float);
    GLuint mi_size = moonIndices.size()*sizeof(int);

    glGenBuffers(1, &moonVBO);
    glGenVertexArrays(1, &moonVAO);

    glBindVertexArray(moonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    glBufferData(GL_ARRAY_BUFFER, mv_size, moonVertices.data(), GL_DYNAMIC_DRAW); // TODO: Static draw???

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(sizeof(float)*3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(sizeof(float)*6));

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

    glGenBuffers(1, &moonEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mi_size, moonIndices.data(), GL_DYNAMIC_DRAW);

    GLint moon_lightPos_id = glGetUniformLocation(moonShaderID, "lightPosition");
    glUniform3fv(moon_lightPos_id, 1, glm::value_ptr(lightPos));

    GLint moon_camPos_id = glGetUniformLocation(moonShaderID, "cameraPosition");
    glUniform3fv(moon_lightPos_id, 1, glm::value_ptr(cameraPosition));

    GLint moon_height_f = glGetUniformLocation(moonShaderID, "heightFactor");
    glUniform1f(moon_height_f, (GLfloat) heightFactor);

    GLint moon_img_w = glGetUniformLocation(moonShaderID, "imageWidth");
    glUniform1f(moon_img_w, (GLfloat) moonImageWidth);

    GLint moon_img_h = glGetUniformLocation(moonShaderID, "imageHeight");
    glUniform1f(moon_img_h, (GLfloat) moonImageHeight);


    glm::mat4 moonModellingMatrix = glm::mat4(1); // TODO:Change???
    glm::mat4 moonNormalMatrix = glm::mat4(1); // TODO:Change???

    GLint moon_pMat_id = glGetUniformLocation(moonShaderID, "ProjectionMatrix");
    glUniformMatrix4fv(moon_pMat_id, 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));

    GLint moon_viewMat_id = glGetUniformLocation(moonShaderID, "ViewMatrix");
    glUniformMatrix4fv(moon_viewMat_id, 1, GL_FALSE, glm::value_ptr(camMatrix));

    GLint moon_normalMat_id = glGetUniformLocation(moonShaderID, "NormalMatrix");
    glUniformMatrix4fv(moon_normalMat_id, 1, GL_FALSE, glm::value_ptr(moonNormalMatrix));

    GLint moon_mvp_id = glGetUniformLocation(moonShaderID, "MVP");
    glUniformMatrix4fv(moon_mvp_id, 1, GL_FALSE, glm::value_ptr(moonModellingMatrix));

    // World commands
    // Load shaders
    GLuint worldShaderID = initShaders("worldShader.vert", "worldShader.frag");

    glActiveTexture(GL_TEXTURE0);
    initColoredTexture(coloredTexturePath, worldShaderID);
    glActiveTexture(GL_TEXTURE1);
    initGreyTexture(greyTexturePath, worldShaderID);

    // Set worldVertices
    createSphere(radius, glm::vec3(0,0,0), worldVertices, worldIndices);

    // Configure Buffers
    GLuint wv_size = worldVertices.size()*sizeof(float);
    GLuint wi_size = worldIndices.size()*sizeof(int);

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, wv_size, worldVertices.data(), GL_DYNAMIC_DRAW); // TODO: Static draw???

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(sizeof(float)*3));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(sizeof(float)*6));

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, wi_size, worldIndices.data(), GL_DYNAMIC_DRAW);

    GLint world_lightPos_id = glGetUniformLocation(worldShaderID, "lightPosition");
    glUniform3fv(world_lightPos_id, 1, glm::value_ptr(lightPos));

    GLint world_camPos_id = glGetUniformLocation(worldShaderID, "cameraPosition");
    glUniform3fv(world_lightPos_id, 1, glm::value_ptr(cameraPosition));

    GLint world_height_f = glGetUniformLocation(worldShaderID, "heightFactor");
    glUniform1f(world_height_f, (GLfloat) heightFactor);

    GLint world_img_w = glGetUniformLocation(worldShaderID, "imageWidth");
    glUniform1f(world_img_w, (GLfloat) imageWidth);

    GLint world_img_h = glGetUniformLocation(worldShaderID, "imageHeight");
    glUniform1f(world_img_h, (GLfloat) imageHeight);


    glm::mat4 worldModellingMatrix = glm::mat4(1); // TODO:Change???
    glm::mat4 worldNormalMatrix = glm::mat4(1); // TODO:Change???

    GLint world_pMat_id = glGetUniformLocation(worldShaderID, "ProjectionMatrix");
    glUniformMatrix4fv(world_pMat_id, 1, GL_FALSE, glm::value_ptr(perspectiveMatrix));

    GLint world_viewMat_id = glGetUniformLocation(worldShaderID, "ViewMatrix");
    glUniformMatrix4fv(world_viewMat_id, 1, GL_FALSE, glm::value_ptr(camMatrix));

    GLint world_normalMat_id = glGetUniformLocation(worldShaderID, "NormalMatrix");
    glUniformMatrix4fv(world_normalMat_id, 1, GL_FALSE, glm::value_ptr(worldNormalMatrix));

    GLint world_mvp_id = glGetUniformLocation(worldShaderID, "MVP");
    glUniformMatrix4fv(world_mvp_id, 1, GL_FALSE, glm::value_ptr(worldModellingMatrix));

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Main rendering loop
    do {
        glViewport(0, 0, screenWidth, screenHeight);

        glClearStencil(0);
        glClearDepth(1.0f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // TODO: Handle key presses
        handleKeyPress(window);

        // TODO: Manipulate rotation variables


        // TODO: Bind textures

        // TODO: Use moonShaderID program
        glUseProgram(moonShaderID);

        // TODO: Update camera at every frame

        // TODO: Update uniform variables at every frame

        // TODO: Bind moon vertex array
        glBindVertexArray(moonVAO);

        // TODO: Draw moon object
        glDrawElements(GL_TRIANGLES, moonVertices.size(), GL_UNSIGNED_INT, (void*)0);
        /*************************/

        // TODO: Use worldShaderID program
        glUseProgram(worldShaderID);

        // TODO: Update camera at every frame

        // TODO: Update uniform variables at every frame

        // TODO: Bind world vertex array
        glBindVertexArray(VAO);

        // TODO: Draw world object
        glDrawElements(GL_TRIANGLES, worldVertices.size(), GL_UNSIGNED_INT, (void*)0);


        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Delete buffers
    glDeleteBuffers(1, &moonVAO);
    glDeleteBuffers(1, &moonVBO);
    glDeleteBuffers(1, &moonEBO);


    // Delete buffers
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glDeleteProgram(moonShaderID);
    glDeleteProgram(worldShaderID);

    // Close window
    glfwTerminate();
}

void EclipseMap::handleKeyPress(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}

GLFWwindow *EclipseMap::openWindow(const char *windowName, int width, int height)
{
    if (!glfwInit()) {
        getchar();
        return 0;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, windowName, NULL, NULL);
    glfwSetWindowMonitor(window, NULL, 1, 31, screenWidth, screenHeight, mode->refreshRate);

    if (window == NULL) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0, 0, 0, 0);

    return window;
}


void EclipseMap::initColoredTexture(const char *filename, GLuint shader)
{
    int width, height;
    glGenTextures(1, &textureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, textureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);


    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexColor"), 0);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

void EclipseMap::initGreyTexture(const char *filename, GLuint shader)
{
    glGenTextures(1, &textureGrey);
    glBindTexture(GL_TEXTURE_2D, textureGrey);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height;

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);




    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "TexGrey"), 1);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

void EclipseMap::initMoonColoredTexture(const char *filename, GLuint shader)
{
    int width, height;
    glGenTextures(1, &moonTextureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, moonTextureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);


    moonImageWidth = width;
    moonImageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    glUniform1i(glGetUniformLocation(shader, "MoonTexColor"), 2);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}
