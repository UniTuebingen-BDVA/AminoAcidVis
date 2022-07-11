/**
 * @file   		Atoms.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		24.04.2017
 *
 * @brief  		Contains the main class that holds and manages most of the needed data, like the atom model and trajectories.
 *
 * @copyright{
 *   AminoAcidVis
 *   Copyright (C) 2017 Vladimir Ageev
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *   USA
 *  }
 */

#ifndef LIBRARIES_ATOMS_ATOMS_H_
#define LIBRARIES_ATOMS_ATOMS_H_

#include <QObject>
#include <QAbstractItemModel>
#include <QString>
#include <QVector>
#include <QList>
#include <QColor>
#include <QDebug>
#include <glm/glm.hpp>
#include <Util/AABB.h>
#include <Atoms/TrajectoryStream.h>

#include <xdrfile_xtc.h>
#include <limits>

class Timeline;

class FilterAtomsListModel;

/*!
 * @brief This class will load and store .pdb (Protein Data bank) and .xtc (trajectories) files.
 * @see mmcif.wwpdb.org
 * @see http://manual.gromacs.org/online/xtc.html
 */
class Atoms : public QAbstractItemModel {
Q_OBJECT
    Q_PROPERTY(unsigned int numberOfAtroms READ numberOfAtroms NOTIFY onModelDataChanged)
    Q_PROPERTY(unsigned int numberOfGroups READ numberOfGroups NOTIFY onModelDataChanged)
    Q_PROPERTY(unsigned int numberOfFrames READ numberOfFrames NOTIFY onFramesChanged)

    Q_PROPERTY(int hoveredAtom READ getHoveredAtom WRITE setHoveredAtom NOTIFY hoveredChanged)
    Q_PROPERTY(int hoveredGroup READ getHoveredGroup WRITE setHoveredGroup NOTIFY hoveredChanged)
    Q_PROPERTY(int selectedAtom READ getSelectedAtom WRITE setSelectedAtom NOTIFY selectionChanged)
    Q_PROPERTY(int selectedGroup READ getSelectedGroup WRITE setSelectedGroup NOTIFY selectionChanged)
public:
    /*!
     * @brief Information about a atom extracted from a .pdb file.
     * Each atom belongs to a residue. A residue is a building block, which connected together as a chain from the amino acid.
     */
    struct atom {
        QString name; /// Name of the atom. This contains information of the atom position inside the residue.
        QString element; /// Just the name of the atom element.
        QString residue; /// Residue name. A residue is one building block of the hole amino acid, which are connected in a chain to form the amino acid.
        unsigned int groupID; /// The residue ID.
        unsigned int proteinID;
        glm::vec3 position; /// Orthogonal position in Angstroms.
        QColor color;
        float radius;
    };

    struct layerFrame {
        int maxLayer = -1;
        QVector<float> layers;
    };

    /// Custom roles for QML data access
    enum CustomRoles {
        TypeRole = Qt::UserRole + 1,
        isAtomRole = Qt::UserRole + 2,
        ItemIndexRole = Qt::UserRole + 3,
        ItemRowRole = Qt::UserRole + 4
    };


    typedef QPair<int, int> bund;

    Atoms(const QString &resourcePath = "resources", QObject *parent = nullptr);

    void setData(Timeline *timeline, FilterAtomsListModel *filters);

    /*!
     * @brief Will open a given .pdb or .gro file depending on the path ending.
     * @see openPBD
     * @see openGRO
     */
    bool openModel(const QString &path);

    /*!
     * @brief Parses a given .PDB protein model file.
     * @returns true, if parsing successful
     * @see getAtoms
     */
    bool openPBD(const QString &path);

    /*!
     * @brief Parses a given .GRO protein model file.
     * http://manual.gromacs.org/documentation/5.1/user-guide/file-formats.html#gro
     * @returns true, if parsing successful
     * @see getAtoms
     */
    bool openGRO(const QString &path);

    /*!
     * @brief Parses a given .XTC trajectories file.
     * @returns true, if parsing successful
     * @see getFrames
     */
    bool openXTC(const QString &path);

    /*!
     * @brief Open both a protein model .PDB/.GRO and a .XTC trajectories file.
     * The trajectories need to have the same number of atoms as the model.
     * @returns true, if parsing successful
     * @see openPBD
     * @see openXTC
     */
    bool open(const QString &modelPath, const QString &xtcPath);

    /*!
     * @brief Can export the atom layer data as .bin (binary format) or as .csv (Comma-separated values).
     */
    Q_INVOKABLE bool exportLayerData(const QString &path, float sasRadius = 1.0) const;
    /*!
     * @brief Can import the exported atom layer data as .bin (binary format) or as .csv (Comma-separated values).
     * @see exportLayerData
     */
    Q_INVOKABLE bool importLayerData(const QString &path, float &sasRadiusOut);

    operator bool() const;

    int numberOfAtroms() const;

    int numberOfGroups() const;

    int numberOfProteins() const;

    int numberOfFrames() const;

    atom &operator[](unsigned int i);

    const atom &operator[](unsigned int i) const;

    atom &getAtom(unsigned int i);

    const atom &getAtom(unsigned int i) const;

    /// @returns The loaded protein model.
    QVector<atom> &getAtoms();

    const QVector<atom> &getAtoms() const;

    TrajectoryStream::xtcFrame &getFrame(unsigned int i);

    TrajectoryStream::xtcFrame &getCurrentFrame();

    const TrajectoryStream::xtcFrame &getCurrentFrame() const;

    Atoms::layerFrame &getLayer(unsigned int i);

    const Atoms::layerFrame &getLayer(unsigned int i) const;

    QVector<Atoms::layerFrame> &getLayers();

    const QVector<Atoms::layerFrame> &getLayers() const;

    inline int getWaterCount() const { return m_waterCount; }

    inline const QVector<unsigned int> &getOffsets() const { return m_frameOffsets; };

    inline const TrajectoryStream &getStream() const { return m_trajectoryStream; };

    const QString &getHeader() const;

    const QString &getTitle() const;

    ///@returns a list with indexes of the first atom in each amino acid
    const QVector<int> &getGroupStartIDs() const;

    ///@returns a list with indexes of the first atom in each protein
    const QVector<int> &getProteinStartIDs() const;

    Q_INVOKABLE QString getAtomName(int index) const;

    Q_INVOKABLE int getGroupStartID(int groupindex) const;

    Q_INVOKABLE QString getGroupName(int index) const;
    ///@returns the number of atoms inside a group
    Q_INVOKABLE int getAtomsInGroup(int groupindex) const;


    ///@returns filtered result from the atoms
    Q_INVOKABLE float getAtomLayer(int atomIndex, int frame, bool applyFilters = true, bool isTimeline = true) const;

    Q_INVOKABLE float getGroupLayerAvarage(int groupIndex, int frame) const;

    Q_INVOKABLE float getGroupLayer(int groupIndex, int frame, bool applyFilters = true, bool isTimeline = true) const;

    float getGroupLayerAvarage(int groupIndex, const Atoms::layerFrame &frame) const;

    void fillGroupLayerAvarage(QVector<float> &avarage, const Atoms::layerFrame &frame) const;

    void fillGroupLayer(QVector<float> &avarage, int frame, bool applyFilters = true, bool isTimeline = true) const;

    void fillAtomLayer(QVector<float> &atomLayers, int frame, bool applyFilters = true, bool isTimeline = true) const;

    QVector<atom>::iterator findAtomInGroup(int groupIndex, const QString &atomName);

    QVector<atom>::const_iterator findAtomInGroup(int groupIndex, const QString &atomName) const;

    QString getFullResidueName(const QString &residue) const;

    bool createBond(int fromResidue, const QString &fromAtomName, int toResidue, const QString &toAtomName);

    const QVector<bund> &getBonds() const;


    int getHoveredAtom() const;

    int getHoveredGroup() const;

    int getSelectedAtom() const;

    int getSelectedGroup() const;

    //List model overwrite
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    virtual ~Atoms();

public slots:

    void setHoveredAtom(int id);

    void setHoveredGroup(int id);

    void setHovered(int atomID, int groupID);

    void setSelectedAtom(int id);

    void setSelectedGroup(int id);

    void clear();

signals:

    void onModelDataChanged();

    void onFramesChanged();

    void hoveredChanged();

    void selectionChanged();

private:
    //interpretation
    QMap<QString, QVector<QPair<QString, QString>>> m_bondsMap; /// Contains the default bonds structure for a residue (residue abbreviation, bounds)
    void readBonds(const QString &file);

    QMap<QString, QString> m_alternativeResidueNames; /// Contains alternative names residues
    QMap<QString, QString> m_residuesType; /// Contains the type of residue, like Protein or Nucleic
    QMap<QString, QMap<QString, QString> > m_alternativeAtomNames; /// Contains alternative names for atroms inside a residue (ResedueName, (AlternativeName, AtomName) )
    void readAltanativePDBNames(const QString &file);

    QMap<QString, QString> m_residuesFullName; /// The full names of the residues (residue abbreviation, full name)
    void readFullNames(const QString &file);

    QMap<QString, QPair<QColor, float>> m_elemetColorsAndVdW; /// Stores a CPK color and Van-der-Waals-Radius for each element
    void readElementColorsAndVdW(const QString &file);

    void completeModel();

    //links
    Timeline *m_timeline = nullptr;
    FilterAtomsListModel *m_filterAtomsListModel = nullptr;

    //base data
    QString m_header;
    QString m_title;
    QVector<atom> m_model;
    QVector<int> m_groupStartIDs; /// Contains the index of the first atom of a residue
    QVector<int> m_proteinStartIDs; /// Contains the index of the first atom of a protein
    QVector<bund> m_bonds;
    int m_waterCount = 0;

    //Streaming frames
    QVector<unsigned int> m_frameOffsets;
    TrajectoryStream m_trajectoryStream;
    QVector<Atoms::layerFrame> m_layers;
    //QList<xtcFrame> m_frames;

    //selection
    int m_hoveredAtom = -1;
    int m_hoveredGroup = -1;
    int m_selectedAtom = -1;
    int m_selectedGroup = -1;
};

QDebug operator<<(QDebug d, const glm::vec2 &m);

QDebug operator<<(QDebug d, const glm::vec3 &m);

QDebug operator<<(QDebug d, const Atoms::atom &m);

QDebug operator<<(QDebug d, const TrajectoryStream::xtcFrame &m);

QDebug operator<<(QDebug d, const Atoms &m);

#endif /* LIBRARIES_ATOMS_ATOMS_H_ */
