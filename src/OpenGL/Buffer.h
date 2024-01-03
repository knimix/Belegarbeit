#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
    struct BufferElement{
        std::string Name;
        unsigned int DataType;
        uint32_t Count;
    };
    static uint32_t DataTypeSize(unsigned int type){
        switch (type) {
            case GL_FLOAT: return 4;
            case GL_INT: return 4;
            case GL_BYTE: return 1;
            case GL_BOOL: return 1;
            default: return 4;
        }
    }
    class  BufferLayout{
    public:
        void Push(const std::string& name,unsigned int type, uint32_t count);
        inline const std::vector<BufferElement>& GetElements() const {return m_Elements;}
        inline uint32_t GetStride() const {return m_Stride;}
    private:
        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride = 0;
    };
    class  VertexBuffer{
    public:
        VertexBuffer();
        VertexBuffer(uint64_t size, unsigned int drawMode);
        VertexBuffer(const void *data, uint64_t size, unsigned int drawMode);
        ~VertexBuffer();
        void Bind() const;
        void Unbind() const;
        void Data(const void *data, uint64_t size, unsigned int drawMode) const;
        void SubData(const void *data, uint64_t size, unsigned int offset)const;
    private:
        unsigned int m_VB = 0;
    };
    class ShaderBuffer{
    public:
        ShaderBuffer();
        ShaderBuffer(uint64_t size, unsigned int drawMode);
        ShaderBuffer(const void *data, uint64_t size, unsigned int drawMode);
        ~ShaderBuffer();
        void Bind() const;
        void Unbind() const;
        void BindBufferBase(int base);
        void Data(const void *data, uint64_t size, unsigned int drawMode) const;
        void SubData(const void *data, uint64_t size, unsigned int offset)const;
    private:
        unsigned int m_SB = 0;
    };
    class  VertexArray{
    public:
        VertexArray();
        ~VertexArray();
        void Bind();
        void Unbind();
        void AddBuffer(const VertexBuffer& buffer, const BufferLayout& layout);
    private:
        unsigned int m_VA = 0;
    };
    class  IndexBuffer{
    public:
        IndexBuffer(const unsigned int *data, uint64_t count, unsigned int drawType);
        ~IndexBuffer();
        void Bind();
        void Unbind();
        void Data(const unsigned int *data, uint64_t count, unsigned int drawType);
        void SubData(const void *data, uint64_t size, unsigned int offset);
    private:
        unsigned int m_IB = 0;
    };
