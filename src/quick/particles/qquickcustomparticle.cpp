/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickcustomparticle_p.h"
#include <QtQuick/private/qquickshadereffectmesh_p.h>
#include <cstdlib>

QT_BEGIN_NAMESPACE

//Includes comments because the code isn't self explanatory
static const char qt_particles_template_vertex_code[] =
        "attribute highp vec2 qt_ParticlePos;\n"
        "attribute highp vec2 qt_ParticleTex;\n"
        "attribute highp vec4 qt_ParticleData; //  x = time,  y = lifeSpan, z = size,  w = endSize\n"
        "attribute highp vec4 qt_ParticleVec; // x,y = constant speed,  z,w = acceleration\n"
        "attribute highp float qt_ParticleR;\n"
        "uniform highp mat4 qt_Matrix;\n"
        "uniform highp float qt_Timestamp;\n"
        "varying highp vec2 qt_TexCoord0;\n"
        "void defaultMain() {\n"
        "    qt_TexCoord0 = qt_ParticleTex;\n"
        "    highp float size = qt_ParticleData.z;\n"
        "    highp float endSize = qt_ParticleData.w;\n"
        "    highp float t = (qt_Timestamp - qt_ParticleData.x) / qt_ParticleData.y;\n"
        "    highp float currentSize = mix(size, endSize, t * t);\n"
        "    if (t < 0. || t > 1.)\n"
        "        currentSize = 0.;\n"
        "    highp vec2 pos = qt_ParticlePos\n"
        "                   - currentSize / 2. + currentSize * qt_ParticleTex   // adjust size\n"
        "                   + qt_ParticleVec.xy * t * qt_ParticleData.y         // apply speed vector..\n"
        "                   + 0.5 * qt_ParticleVec.zw * pow(t * qt_ParticleData.y, 2.);\n"
        "    gl_Position = qt_Matrix * vec4(pos.x, pos.y, 0, 1);\n"
        "}";
static const char qt_particles_default_vertex_code[] =
        "void main() {        \n"
        "    defaultMain();   \n"
        "}";

static const char qt_particles_default_fragment_code[] =
        "uniform sampler2D source;                                  \n"
        "varying highp vec2 qt_TexCoord0;                           \n"
        "uniform lowp float qt_Opacity;                             \n"
        "void main() {                                              \n"
        "    gl_FragColor = texture2D(source, qt_TexCoord0) * qt_Opacity;   \n"
        "}";

static QSGGeometry::Attribute PlainParticle_Attributes[] = {
    QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),       // Position
    QSGGeometry::Attribute::create(1, 2, GL_FLOAT),             // TexCoord
    QSGGeometry::Attribute::create(2, 4, GL_FLOAT),             // Data
    QSGGeometry::Attribute::create(3, 4, GL_FLOAT),             // Vectors
    QSGGeometry::Attribute::create(4, 1, GL_FLOAT)              // r
};

static QSGGeometry::AttributeSet PlainParticle_AttributeSet =
{
    5, // Attribute Count
    (2 + 2 + 4 + 4 + 1) * sizeof(float),
    PlainParticle_Attributes
};

struct PlainVertex {
    float x;
    float y;
    float tx;
    float ty;
    float t;
    float lifeSpan;
    float size;
    float endSize;
    float vx;
    float vy;
    float ax;
    float ay;
    float r;
};

struct PlainVertices {
    PlainVertex v1;
    PlainVertex v2;
    PlainVertex v3;
    PlainVertex v4;
};

/*!
    \qmlclass CustomParticle QQuickCustomParticle
    \inqmlmodule QtQuick.Particles 2
    \inherits ParticlePainter
    \brief The CustomParticle element allows you to specify your own shader to paint particles.

*/

QQuickCustomParticle::QQuickCustomParticle(QQuickItem* parent)
    : QQuickParticlePainter(parent)
    , m_dirtyUniforms(true)
    , m_dirtyUniformValues(true)
    , m_dirtyTextureProviders(true)
    , m_dirtyProgram(true)
{
    setFlag(QQuickItem::ItemHasContents);
}

void QQuickCustomParticle::sceneGraphInvalidated()
{
    m_nodes.clear();
}

QQuickCustomParticle::~QQuickCustomParticle()
{
}

void QQuickCustomParticle::componentComplete()
{
    m_common.updateShader(this, Key::FragmentShader);
    updateVertexShader();
    reset();
    QQuickParticlePainter::componentComplete();
}


//Trying to keep the shader conventions the same as in qsgshadereffectitem
/*!
    \qmlproperty string QtQuick.Particles2::CustomParticle::fragmentShader

    This property holds the fragment shader's GLSL source code.
    The default shader expects the texture coordinate to be passed from the
    vertex shader as "varying highp vec2 qt_TexCoord0", and it samples from a
    sampler2D named "source".
*/

void QQuickCustomParticle::setFragmentShader(const QByteArray &code)
{
    if (m_common.source.sourceCode[Key::FragmentShader].constData() == code.constData())
        return;
    m_common.source.sourceCode[Key::FragmentShader] = code;
    m_dirtyProgram = true;
    if (isComponentComplete()) {
        m_common.updateShader(this, Key::FragmentShader);
        reset();
    }
    emit fragmentShaderChanged();
}

/*!
    \qmlproperty string QtQuick.Particles2::CustomParticle::vertexShader

    This property holds the vertex shader's GLSL source code.

    The default shader passes the texture coordinate along to the fragment
    shader as "varying highp vec2 qt_TexCoord0".

    To aid writing a particle vertex shader, the following GLSL code is prepended
    to your vertex shader:
    \code
        attribute highp vec2 qt_ParticlePos;
        attribute highp vec2 qt_ParticleTex;
        attribute highp vec4 qt_ParticleData; //  x = time,  y = lifeSpan, z = size,  w = endSize
        attribute highp vec4 qt_ParticleVec; // x,y = constant speed,  z,w = acceleration
        attribute highp float qt_ParticleR;
        uniform highp mat4 qt_Matrix;
        uniform highp float qt_Timestamp;
        varying highp vec2 qt_TexCoord0;
        void defaultMain() {
            qt_TexCoord0 = qt_ParticleTex;
            highp float size = qt_ParticleData.z;
            highp float endSize = qt_ParticleData.w;
            highp float t = (qt_Timestamp - qt_ParticleData.x) / qt_ParticleData.y;
            highp float currentSize = mix(size, endSize, t * t);
            if (t < 0. || t > 1.)
                currentSize = 0.;
            highp vec2 pos = qt_ParticlePos
                           - currentSize / 2. + currentSize * qt_ParticleTex   // adjust size
                           + qt_ParticleVec.xy * t * qt_ParticleData.y         // apply speed vector..
                           + 0.5 * qt_ParticleVec.zw * pow(t * qt_ParticleData.y, 2.);
            gl_Position = qt_Matrix * vec4(pos.x, pos.y, 0, 1);
        }
    \endcode

    defaultMain() is the same code as in the default shader, you can call this for basic
    particle functions and then add additional variables for custom effects. Note that
    the vertex shader for particles is responsible for simulating the movement of particles
    over time, the particle data itself only has the starting position and spawn time.
*/

void QQuickCustomParticle::setVertexShader(const QByteArray &code)
{
    if (m_common.source.sourceCode[Key::VertexShader].constData() == code.constData())
        return;
    m_common.source.sourceCode[Key::VertexShader] = code;

    m_dirtyProgram = true;
    if (isComponentComplete()) {
        updateVertexShader();
        reset();
    }
    emit vertexShaderChanged();
}

void QQuickCustomParticle::updateVertexShader()
{
    m_common.disconnectPropertySignals(this, Key::VertexShader);
    qDeleteAll(m_common.signalMappers[Key::VertexShader]);
    m_common.uniformData[Key::VertexShader].clear();
    m_common.signalMappers[Key::VertexShader].clear();
    m_common.attributes.clear();
    m_common.attributes.append("qt_ParticlePos");
    m_common.attributes.append("qt_ParticleTex");
    m_common.attributes.append("qt_ParticleData");
    m_common.attributes.append("qt_ParticleVec");
    m_common.attributes.append("qt_ParticleR");

    UniformData d;
    d.name = "qt_Matrix";
    d.specialType = UniformData::Matrix;
    m_common.uniformData[Key::VertexShader].append(d);
    m_common.signalMappers[Key::VertexShader].append(0);

    d.name = "qt_Timestamp";
    d.specialType = UniformData::None;
    m_common.uniformData[Key::VertexShader].append(d);
    m_common.signalMappers[Key::VertexShader].append(0);

    const QByteArray &code = m_common.source.sourceCode[Key::VertexShader];
    if (!code.isEmpty())
        m_common.lookThroughShaderCode(this, Key::VertexShader, code);

    m_common.connectPropertySignals(this, Key::VertexShader);
}

void QQuickCustomParticle::reset()
{
    QQuickParticlePainter::reset();
    update();
}

QSGNode *QQuickCustomParticle::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QQuickShaderEffectNode *rootNode = static_cast<QQuickShaderEffectNode *>(oldNode);
    if (m_pleaseReset){
        delete rootNode;//Automatically deletes children
        rootNode = 0;
        m_nodes.clear();
        m_pleaseReset = false;
        m_dirtyProgram = true;
    }

    if (m_system && m_system->isRunning() && !m_system->isPaused()){
        rootNode = prepareNextFrame(rootNode);
        if (rootNode)
            update();
    }

    return rootNode;
}

QQuickShaderEffectNode *QQuickCustomParticle::prepareNextFrame(QQuickShaderEffectNode *rootNode)
{
    if (!rootNode)
        rootNode = buildCustomNodes();

    if (!rootNode)
        return 0;

    if (m_dirtyProgram) {
        QQuickShaderEffectMaterial *material = static_cast<QQuickShaderEffectMaterial *>(rootNode->material());
        Q_ASSERT(material);

        Key s = m_common.source;
        if (s.sourceCode[Key::FragmentShader].isEmpty())
            s.sourceCode[Key::FragmentShader] = qt_particles_default_fragment_code;
        if (s.sourceCode[Key::VertexShader].isEmpty())
            s.sourceCode[Key::VertexShader] = qt_particles_default_vertex_code;
        s.sourceCode[Key::VertexShader] = qt_particles_template_vertex_code + s.sourceCode[Key::VertexShader];
        s.className = metaObject()->className();

        material->setProgramSource(s);
        material->attributes = m_common.attributes;
        foreach (QQuickShaderEffectNode* node, m_nodes)
            node->markDirty(QSGNode::DirtyMaterial);

        m_dirtyProgram = false;
        m_dirtyUniforms = true;
    }

    m_lastTime = m_system->systemSync(this) / 1000.;
    if (true) //Currently this is how we update timestamp... potentially over expensive.
        buildData(rootNode);
    return rootNode;
}

QQuickShaderEffectNode* QQuickCustomParticle::buildCustomNodes()
{
#ifdef QT_OPENGL_ES_2
    if (m_count * 4 > 0xffff) {
        printf("CustomParticle: Too many particles... \n");
        return 0;
    }
#endif

    if (m_count <= 0) {
        printf("CustomParticle: Too few particles... \n");
        return 0;
    }

    if (m_groups.isEmpty())
        return 0;

    QQuickShaderEffectNode *rootNode = 0;
    QQuickShaderEffectMaterial *material = new QQuickShaderEffectMaterial;
    m_dirtyProgram = true;

    foreach (const QString &str, m_groups){
        int gIdx = m_system->groupIds[str];
        int count = m_system->groupData[gIdx]->size();

        QQuickShaderEffectNode* node = new QQuickShaderEffectNode();
        m_nodes.insert(gIdx, node);

        node->setMaterial(material);

        //Create Particle Geometry
        int vCount = count * 4;
        int iCount = count * 6;
        QSGGeometry *g = new QSGGeometry(PlainParticle_AttributeSet, vCount, iCount);
        g->setDrawingMode(GL_TRIANGLES);
        node->setGeometry(g);
        node->setFlag(QSGNode::OwnsGeometry, true);
        PlainVertex *vertices = (PlainVertex *) g->vertexData();
        for (int p=0; p < count; ++p) {
            commit(gIdx, p);
            vertices[0].tx = 0;
            vertices[0].ty = 0;

            vertices[1].tx = 1;
            vertices[1].ty = 0;

            vertices[2].tx = 0;
            vertices[2].ty = 1;

            vertices[3].tx = 1;
            vertices[3].ty = 1;
            vertices += 4;
        }
        quint16 *indices = g->indexDataAsUShort();
        for (int i=0; i < count; ++i) {
            int o = i * 4;
            indices[0] = o;
            indices[1] = o + 1;
            indices[2] = o + 2;
            indices[3] = o + 1;
            indices[4] = o + 3;
            indices[5] = o + 2;
            indices += 6;
        }
    }

    QHash<int, QQuickShaderEffectNode*>::const_iterator it = m_nodes.begin();
    rootNode = it.value();
    rootNode->setFlag(QSGNode::OwnsMaterial, true);
    for (++it; it != m_nodes.end(); ++it)
        rootNode->appendChildNode(it.value());

    return rootNode;
}

void QQuickCustomParticle::sourceDestroyed(QObject *object)
{
    m_common.sourceDestroyed(object);
}

void QQuickCustomParticle::propertyChanged(int mappedId)
{
    bool textureProviderChanged;
    m_common.propertyChanged(this, mappedId, &textureProviderChanged);
    m_dirtyTextureProviders |= textureProviderChanged;
    m_dirtyUniformValues = true;
    update();
}


void QQuickCustomParticle::buildData(QQuickShaderEffectNode *rootNode)
{
    if (!rootNode)
        return;
    for (int shaderType = 0; shaderType < Key::ShaderTypeCount; ++shaderType) {
        for (int i = 0; i < m_common.uniformData[shaderType].size(); ++i) {
            if (m_common.uniformData[shaderType].at(i).name == "qt_Timestamp")
                m_common.uniformData[shaderType][i].value = qVariantFromValue(m_lastTime);
        }
    }
    m_common.updateMaterial(rootNode, static_cast<QQuickShaderEffectMaterial *>(rootNode->material()),
                            m_dirtyUniforms, true, m_dirtyTextureProviders);
    foreach (QQuickShaderEffectNode* node, m_nodes)
        node->markDirty(QSGNode::DirtyMaterial);
    m_dirtyUniforms = m_dirtyUniformValues = m_dirtyTextureProviders = false;
}

void QQuickCustomParticle::initialize(int gIdx, int pIdx)
{
    QQuickParticleData* datum = m_system->groupData[gIdx]->data[pIdx];
    datum->r = rand()/(qreal)RAND_MAX;
}

void QQuickCustomParticle::commit(int gIdx, int pIdx)
{
    if (m_nodes[gIdx] == 0)
        return;

    QQuickParticleData* datum = m_system->groupData[gIdx]->data[pIdx];
    PlainVertices *particles = (PlainVertices *) m_nodes[gIdx]->geometry()->vertexData();
    PlainVertex *vertices = (PlainVertex *)&particles[pIdx];
    for (int i=0; i<4; ++i) {
        vertices[i].x = datum->x - m_systemOffset.x();
        vertices[i].y = datum->y - m_systemOffset.y();
        vertices[i].t = datum->t;
        vertices[i].lifeSpan = datum->lifeSpan;
        vertices[i].size = datum->size;
        vertices[i].endSize = datum->endSize;
        vertices[i].vx = datum->vx;
        vertices[i].vy = datum->vy;
        vertices[i].ax = datum->ax;
        vertices[i].ay = datum->ay;
        vertices[i].r = datum->r;
    }
}

void QQuickCustomParticle::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange)
        m_common.updateCanvas(value.canvas);
    QQuickParticlePainter::itemChange(change, value);
}


QT_END_NAMESPACE
