#include "Buffer.h"

void BufferLayout::Push(const std::string &name,unsigned int type, uint32_t count) {
    m_Elements.emplace_back(BufferElement{name,type,count});
    m_Stride += count * DataTypeSize(type);
}
VertexBuffer::VertexBuffer() {
    glGenBuffers(1, &m_VB);
    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
}
VertexBuffer::VertexBuffer(uint64_t size, unsigned int drawMode) {
    glGenBuffers(1, &m_VB);
    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, drawMode);
}
VertexBuffer::VertexBuffer(const void *data, uint64_t size, unsigned int drawMode) {
    glGenBuffers(1, &m_VB);
    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
    glBufferData(GL_ARRAY_BUFFER, size, data, drawMode);
}
VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_VB);
}

void VertexBuffer::Bind() const{
    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
}

void VertexBuffer::Unbind() const{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VertexBuffer::Data(const void *data, uint64_t size, unsigned int drawMode) const{
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size,data,drawMode);
}
void VertexBuffer::SubData(const void *data, uint64_t size, unsigned int offset) const{
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER,offset,size,data);
}


ShaderBuffer::ShaderBuffer() {
    glGenBuffers(1, &m_SB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SB);
}
ShaderBuffer::ShaderBuffer(uint64_t size, unsigned int drawMode) {
    glGenBuffers(1, &m_SB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, drawMode);
}

ShaderBuffer::ShaderBuffer(const void *data, uint64_t size, unsigned int drawMode) {
    glGenBuffers(1, &m_SB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, drawMode);
}

ShaderBuffer::~ShaderBuffer() {
    glDeleteBuffers(1, &m_SB);
}

void ShaderBuffer::Bind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SB);
}

void ShaderBuffer::Unbind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void ShaderBuffer::BindBufferBase(int base) {
    Bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, base, m_SB);
}
void ShaderBuffer::Data(const void *data, uint64_t size, unsigned int drawMode) const {
    Bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, drawMode);
}

void ShaderBuffer::SubData(const void *data, uint64_t size, unsigned int offset) const {
    Bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,offset,size,data);
}

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_VA);
    glBindVertexArray(m_VA);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_VA);
}

void VertexArray::Bind() {
    glBindVertexArray(m_VA);
}

void VertexArray::Unbind() {
    glBindVertexArray(0);
}

void VertexArray::AddBuffer(const VertexBuffer& buffer, const BufferLayout& layout) {
    Bind();
    buffer.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for(int i = 0; i< elements.size();i++){
        const BufferElement& element = elements.at(i);
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.Count, element.DataType, false, layout.GetStride(),reinterpret_cast<const void *>(offset));
        offset+= element.Count * DataTypeSize(element.DataType);
    }
}

IndexBuffer::IndexBuffer(const unsigned int *data, uint64_t count, unsigned int drawType) {
    glGenBuffers(1, &m_IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, drawType);
}

IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_IB);
}

void IndexBuffer::Bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IB);
}

void IndexBuffer::Unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::Data(const unsigned int *data, uint64_t count, unsigned int drawType) {
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, drawType);
}

void IndexBuffer::SubData(const void *data, uint64_t size, unsigned int offset) {
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
}
