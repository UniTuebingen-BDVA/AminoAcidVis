/*
 * Atoms.cpp
 *
 *  Created on: 24.04.2017
 *      Author: Vladimir Ageev (vladimir.agueev@progsys.de
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

#include <Atoms/Atoms.h>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDataStream>
#include <QXmlStreamReader>

#include <QMessageBox>
#include <QApplication>

#include <Atoms/Timeline.h>
#include <Atoms/FilterAtoms.h>
#include <Atoms/FilterNode.h>
#include <Atoms/FilterDefinitions.h>

void Atoms::readAltanativePDBNames(const QString &file) {
    m_alternativeResidueNames.clear();
    m_residuesType.clear();

    //read names
    QFile xmlFile(file);
    if (xmlFile.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&xmlFile);
        QString residueName;

        while (!xml.atEnd()) {
            //read next token
            QXmlStreamReader::TokenType token = xml.readNext();

            // error handling
            if (xml.hasError()) {
                QMessageBox::critical(nullptr,
                                      "XML Parse Error", "File: " + file + "\nError:" + xml.errorString(),
                                      QMessageBox::Ok);
                return;
            }

            if (token == QXmlStreamReader::TokenType::StartElement) {
                QString name = xml.name().toString();

                if (name == "Residue") { // Residue Tag
                    //Parameters
                    QString type;
                    QList<QString> altNames;

                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "name") {
                            residueName = att.value().toString();
                        } else if (att.name() == "type") {
                            type = att.value().toString();
                        } else if (att.name().startsWith("alt")) {
                            altNames.push_back(att.value().toString());
                        }
                    }
                    //error handling
                    if (residueName.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error",
                                              "File: " + file + "\nError: Residue name not defined at " +
                                              QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }
                    //store data
                    if (!type.isEmpty()) m_residuesType[residueName] = type;

                    for (const QString &r: altNames) {
                        m_alternativeResidueNames[r] = residueName;
                    }

                } else if (name == "Atom") {  // Atom Tag
                    //Parameters
                    QString name;
                    QList<QString> altNames;

                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "name") {
                            name = att.value().toString();
                        } else if (att.name().startsWith("alt")) {
                            altNames.push_back(att.value().toString());
                        }
                    }
                    //error handling
                    if (residueName.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error",
                                              "File: " + file + "\nError: Residue must have a 'name' attribute, at  " +
                                              QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                    if (name.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error",
                                              "File: " + file + "\nError: Atom tag must have a 'name' attribute, at " +
                                              QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                    //store data
                    for (const QString &r: altNames) {
                        m_alternativeAtomNames[residueName][r] = name;
                    }
                }
            } else if (token == QXmlStreamReader::TokenType::EndElement) {
                if (xml.name() == "Residue") residueName.clear();
            }
        } //while loop
    }
    xmlFile.close();
}

void Atoms::readBonds(const QString &file) {
    m_bondsMap.clear();

    //read names
    QFile xmlFile(file);
    if (xmlFile.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&xmlFile);
        QString residueName;
        while (!xml.atEnd()) {
            //read next token
            QXmlStreamReader::TokenType token = xml.readNext();

            // error handling
            if (xml.hasError()) {
                QMessageBox::critical(nullptr,
                                      "XML Parse Error", "File: " + file + "\nError:" + xml.errorString(),
                                      QMessageBox::Ok);
                return;
            }

            if (token == QXmlStreamReader::TokenType::StartElement) {
                QString name = xml.name().toString();

                if (name == "Residue") { // Residue Tag
                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "name") {
                            residueName = att.value().toString();
                        }
                    }
                    //error handling
                    if (residueName.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error",
                                              "File: " + file + "\nError: Residue must have a 'name' attribute, at " +
                                              QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                } else if (name == "Bond") {  // Atom Tag
                    //Parameters
                    QString from;
                    QString to;

                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "from") {
                            from = att.value().toString();
                        } else if (att.name() == "to") {
                            to = att.value().toString();
                        }
                    }
                    //error handling
                    if (residueName.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error", "File: " + file +
                                                                 "\nError: 'Bond' tag must be inside 'Residue' tag at  " +
                                                                 QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                    if (from.isEmpty() || from.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error", "File: " + file +
                                                                 "\nError: Bond tag must have a 'from' and 'to' attribute, at " +
                                                                 QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                    //store data
                    m_bondsMap[residueName].push_back({from, to});
                }
            } else if (token == QXmlStreamReader::TokenType::EndElement) {
                if (xml.name() == "Residue") residueName.clear();
            }
        } //while loop
    }
    xmlFile.close();
}

void Atoms::readFullNames(const QString &file) {
    m_residuesFullName.clear();

    //read names
    QFile xmlFile(file);
    if (xmlFile.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&xmlFile);
        while (!xml.atEnd()) {
            //read next token
            QXmlStreamReader::TokenType token = xml.readNext();

            // error handling
            if (xml.hasError()) {
                QMessageBox::critical(nullptr,
                                      "XML Parse Error", "File: " + file + "\nError:" + xml.errorString(),
                                      QMessageBox::Ok);
                return;
            }

            if (token == QXmlStreamReader::TokenType::StartElement) {
                QString name = xml.name().toString();

                if (name == "Residue") { // Residue Tag
                    QString shortName;
                    QString name;
                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "name") {
                            name = att.value().toString();
                        } else if (att.name() == "short") {
                            shortName = att.value().toString();
                        }
                    }

                    if (shortName.isEmpty() || name.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error", "File: " + file +
                                                                 "\nError: Residue tag must have a 'short' and 'name' attribute, at " +
                                                                 QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }

                    m_residuesFullName[shortName] = name;
                }
            }
        } //while loop
    }
    xmlFile.close();
}

void Atoms::readElementColorsAndVdW(const QString &file) {
    m_elemetColorsAndVdW.clear();

    //read names
    QFile xmlFile(file);
    if (xmlFile.open(QIODevice::ReadOnly)) {
        QXmlStreamReader xml(&xmlFile);
        while (!xml.atEnd()) {
            //read next token
            QXmlStreamReader::TokenType token = xml.readNext();

            // error handling
            if (xml.hasError()) {
                QMessageBox::critical(nullptr,
                                      "XML Parse Error", "File: " + file + "\nError:" + xml.errorString(),
                                      QMessageBox::Ok);
                return;
            }

            if (token == QXmlStreamReader::TokenType::StartElement) {
                QString name = xml.name().toString();

                if (name == "Element") { // Residue Tag
                    QString color("#ffffff");
                    QString name;
                    int vdw = 100;
                    //read all attributes
                    for (const QXmlStreamAttribute &att: xml.attributes()) {
                        if (att.name() == "name") {
                            name = att.value().toString();
                        } else if (att.name() == "color") {
                            color = att.value().toString();
                        } else if (att.name() == "vdw") {
                            vdw = att.value().toInt();
                        }
                    }

                    if (name.isEmpty()) {
                        QMessageBox::critical(nullptr,
                                              "XML Parse Error", "File: " + file +
                                                                 "\nError: Element tag must have a 'name' attribute, at " +
                                                                 QString::number(xml.lineNumber()) + "!",
                                              QMessageBox::Ok);
                        return;
                    }
                    m_elemetColorsAndVdW[name] = {QColor(color), vdw / ((float) 100)};
                }
            }
        } //while loop
    }
    xmlFile.close();
}

void Atoms::completeModel() {
    // fix names to default names
    for (atom &a: m_model) {
        //use default residue name
        auto itAltResName = m_alternativeResidueNames.find(a.residue);
        if (itAltResName != m_alternativeResidueNames.end()) a.residue = itAltResName.value();

        //use default atom name
        QString type;
        auto itType = m_residuesType.find(a.residue);
        if (itType != m_residuesType.end()) type = itType.value();

        if (!type.isEmpty()) { //fix name by residue type
            auto it = m_alternativeAtomNames.find(type);
            if (it != m_alternativeAtomNames.end()) {
                auto itAltAtomName = it.value().find(a.name);
                if (itAltAtomName != it.value().end()) a.name = itAltAtomName.value();
            }
        }

        auto it2 = m_alternativeAtomNames.find(a.residue); //fix name by residue name
        if (it2 != m_alternativeAtomNames.end()) {
            auto it3 = it2.value().find(a.name);
            if (it3 != it2.value().end()) a.name = it3.value();
        }
    }

    //build bounds
    //https://github.com/mdtraj/mdtraj/blob/74ea04dfc6c356cb1c5cd3c2b8944f9be745cefa/mdtraj/core/topology.py#L790
    m_bonds.clear();
    for (int groupIndex = 0; groupIndex < m_groupStartIDs.size(); groupIndex++) {
        auto itBondResidue = m_bondsMap.find(m_model[m_groupStartIDs[groupIndex]].residue);
        if (itBondResidue != m_bondsMap.end()) {
            for (const QPair<QString, QString> &bond: itBondResidue.value()) { // go over all possible bounds
                int targetResidues[2]; // from, to
                QString targetAtoms[2]; // from, to

                if (bond.first.startsWith("-")) {
                    targetResidues[0] = groupIndex - 1;
                    targetAtoms[0] = bond.first.mid(1);
                } else if (bond.first.startsWith("+")) {
                    targetResidues[0] = groupIndex + 1;
                    targetAtoms[0] = bond.first.mid(1);
                } else {
                    targetResidues[0] = groupIndex;
                    targetAtoms[0] = bond.first;
                }

                if (bond.second.startsWith("-")) {
                    targetResidues[1] = groupIndex - 1;
                    targetAtoms[1] = bond.second.mid(1);
                } else if (bond.second.startsWith("+")) {
                    targetResidues[1] = groupIndex + 1;
                    targetAtoms[1] = bond.second.mid(1);
                } else {
                    targetResidues[1] = groupIndex;
                    targetAtoms[1] = bond.second;
                }
                //qDebug()<<"createBond"<<targetResidues[0] <<", "<< targetAtoms[0] <<" - "<< targetResidues[1]<<", " << targetAtoms[1]<<" ("<<m_model[m_groupStartIDs[groupIndex]].residue<<", "<<m_groupStartIDs.size()<<")";
                createBond(targetResidues[0], targetAtoms[0], targetResidues[1], targetAtoms[1]);

            }
        }

    }

    qDebug() << "Model completed.";

    m_hoveredAtom = -1;
    m_hoveredGroup = -1;
    m_selectedAtom = -1;
    m_selectedGroup = -1;
    emit hoveredChanged();
    emit onModelDataChanged();
}

Atoms::Atoms(const QString &resourcePath, QObject *parent) : QAbstractItemModel(parent), m_trajectoryStream(this) {
    readAltanativePDBNames(resourcePath + "/data/pdbNames.xml");
    readBonds(resourcePath + "/data/residues.xml");
    readFullNames(resourcePath + "/data/fullNames.xml");
    readElementColorsAndVdW(resourcePath + "/data/cpkAndVdW.xml");
}

void Atoms::setData(Timeline *timeline, FilterAtomsListModel *filters) {
    m_timeline = timeline;
    m_filterAtomsListModel = filters;
}


#define PDB_ERROR(x)  qDebug()<<"["<<__LINE__<<" OpenPDB ERROR at line "<<line_number<<"]: "<<x
#define GRO_ERROR(x)  qDebug()<<"["<<__LINE__<<" OpenGRO ERROR at line "<<line_number+2<<"]: "<<x

bool Atoms::openModel(const QString &path) {
    QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == "pdb")
        return openPBD(path);
    if (suffix == "gro")
        return openGRO(path);
    return false;
}

bool Atoms::openPBD(const QString &path) {

    if (path.isEmpty()) return false;
    QFile file(path); //open the given file
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "File dosn't exists or is protected: " << path;
        return false;
    }
    QAbstractItemModel::beginResetModel();
    clear();
    QAbstractItemModel::endResetModel();
    QApplication::processEvents();
    QAbstractItemModel::beginResetModel();

    bool modelMode = false; //are we inside a model TAG
    int currentGroup = -1;
    QTextStream in(&file);
    unsigned int lastReadGroupID = -1;
    unsigned int groupID = 0;
    unsigned int line_number = 0;
    unsigned int protainID = 0;
    unsigned int currentProtainID = 9999999;
    while (!in.atEnd()) {
        QApplication::processEvents();

        QString line = in.readLine().trimmed();
        line_number++;

        if (line.isEmpty()) continue;
        if (line.startsWith("END")) break;
        if (line.startsWith("REMARK") || line.startsWith("CRYST1")) continue;

        if (line.startsWith("MODEL")) { //model tag start
            modelMode = true;
            m_model.clear();
            m_layers.clear();
            m_groupStartIDs.clear();
            m_proteinStartIDs.clear();
            continue;
        }

        if (line.startsWith("ENDMDL")) {
            if (!modelMode) {
                PDB_ERROR("ENDMDL encountered before MODEL!");
                file.close();
                clear();
                QAbstractItemModel::endResetModel();
                return false;
            }
            modelMode = false;
            continue;
        }

        if (line.startsWith("HEADER")) {
            if (line.size() > 6) m_header = line.mid(6).trimmed();
            continue;
        }

        if (line.startsWith("TITLE")) {
            if (line.size() > 6) m_title = line.mid(5).trimmed();
            continue;
        }

        if (line.startsWith("TER")) {
            protainID++;
            continue;
        }

        if (line.startsWith("ATOM")) {
            /*
            if(!modelMode){
                qDebug()<<"ATOM encountered witch isn't inside a MODEL! "<<path;
                clear();
                file.close();
                return false;
            }*/
            const QStringList parameters = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

            if (parameters.size() != 11 && parameters.size() != 12) {
                PDB_ERROR("NOT enough parameters inside a ATOM tag! (There should be 12 or 11 but there are "
                                  << parameters.size() << ")");
                clear();
                file.close();
                QAbstractItemModel::endResetModel();
                return false;
            }

            QPair<QColor, float> atomInfo = m_elemetColorsAndVdW.value(parameters[parameters.size() - 1],
                                                                       {QColor("#d985f5"), 1});

            //make group ID always increase and unique
            const unsigned int readGroupID = parameters[5].toUInt();
            if (readGroupID != lastReadGroupID) {
                lastReadGroupID = readGroupID;
                groupID++;
            }
            //we just push the parameters and cast them if needed
            m_model.push_back(
                    {
                            parameters[2], parameters[parameters.size() - 1], parameters[3], groupID, protainID,
                            glm::vec3(parameters[6].toFloat(), parameters[7].toFloat(), parameters[8].toFloat()),
                            atomInfo.first, atomInfo.second
                    });
            if (currentGroup != (int) m_model.last().groupID) {
                m_groupStartIDs.push_back(m_model.size() - 1);
                currentGroup = (int) m_model.last().groupID;
                if (m_model.last().residue == "SOL" || m_model.last().residue == "HOH" ||
                    m_model.last().residue.toLower() == "water")
                    m_waterCount++;
            }
            if (currentProtainID != protainID) {
                m_proteinStartIDs.push_back(m_model.size() - 1);
                currentProtainID = protainID;
            }

        }
        continue;
    }

    file.close();

    //set default title if none found
    if (m_title.isEmpty()) m_title = QFileInfo(path).fileName();
    qDebug() << "Opened PBD: " << m_title;

    completeModel();
    return true;
}

bool Atoms::openGRO(const QString &path) {
    if (path.isEmpty()) return false;
    QFile file(path); //open the given file
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "File dosn't exists or is protected: " << path;
        return false;
    }

    QAbstractItemModel::beginResetModel();
    clear();
    QAbstractItemModel::endResetModel();
    QApplication::processEvents();
    QAbstractItemModel::beginResetModel();

    QTextStream in(&file);
    unsigned int currentGroupID = -1;
    unsigned int line_number = 0;

    m_title = in.readLine().trimmed();
    const unsigned int size = in.readLine().trimmed().toUInt();

    m_proteinStartIDs.push_back(0);
    while (line_number < size && !in.atEnd()) {
        QApplication::processEvents();
        const QString line = in.readLine();
        if (line.isEmpty()) continue;
        if (line.size() != 68) {
            GRO_ERROR("Line should have exactly 68 chars!");
            QAbstractItemModel::endResetModel();
            return false;
        }

        line_number++;

        //0:group, 1:amino acid name, 2:atom name, 3:counter, 4:x, 5:y, 6:z, n:? , n:? , n:?,
        const QStringList parameters = {line.mid(0, 5).trimmed(), line.mid(5, 5).trimmed(), line.mid(10, 5).trimmed(),
                                        line.mid(15, 5).trimmed(),
                                        line.mid(20, 8).trimmed(), line.mid(28, 8).trimmed(),
                                        line.mid(36, 8).trimmed()};

        const QString element = parameters[2].left(1);
        const QPair<QColor, float> atomInfo = m_elemetColorsAndVdW.value(element, {QColor("#d985f5"), 1});

        bool ok;
        const unsigned int groupID = parameters[0].toUInt(&ok);
        if (!ok) {
            GRO_ERROR("Failed to convert string to Int!");
            return false;
        }
        //qDebug()<<parameters[4]<<parameters[5]<<parameters[6];
        m_model.push_back(
                {
                        parameters[2], element, parameters[1], groupID, 0,
                        glm::vec3(parameters[4].toFloat() * 10.f, parameters[5].toFloat() * 10.f,
                                  parameters[6].toFloat() * 10.f),
                        atomInfo.first, atomInfo.second
                });

        if (groupID != currentGroupID) {
            m_groupStartIDs.push_back(m_model.size() - 1);
            currentGroupID = groupID;
            if (m_model.last().residue == "SOL" || m_model.last().residue == "HOH" ||
                m_model.last().residue.toLower() == "water") {
                m_waterCount++;
            }
        }
    }

    //set default title if none found
    if (m_title.isEmpty()) m_title = QFileInfo(path).fileName();
    qDebug() << "Opened GRO: " << m_title;

    completeModel();

    return true;
}

bool Atoms::openXTC(const QString &path) {
    if (path.isEmpty() || m_model.isEmpty()) {
        qDebug() << "[ERROR]: XTC filepath is empty! ";
        QAbstractItemModel::endResetModel();
        clear();
        return false;
    }

    m_timeline->reset();

    int numberOfAtroms = 0;
    if (read_xtc_natoms(path.toLatin1(), &numberOfAtroms) != exdrOK) {
        qDebug() << "[ERROR]: Failed to read the number of atoms inside XTC: " << path;
        QAbstractItemModel::endResetModel();
        clear();
        return false;
    }

    if (numberOfAtroms != m_model.size()) {
        qDebug() << "[ERROR]: The number of atoms inside the XTC isn't the same as in the model. (" << numberOfAtroms
                 << " != " << m_model.size() << ")" << path;
        QAbstractItemModel::endResetModel();
        clear();
        return false;
    }


    m_layers.clear();
    m_frameOffsets.clear();

    //reading each frame offset
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);

        while (!stream.atEnd()) {
            QApplication::processEvents();
            int magic;
            stream >> magic;
            if (magic != 1995) break;

            m_frameOffsets.push_back(file.pos() - sizeof(int));
            stream.skipRawData(84);

            unsigned int blockSize;
            stream >> blockSize;
            const unsigned int remainder = blockSize % sizeof(int);
            if (remainder) blockSize += sizeof(int) - remainder;
            stream.skipRawData(blockSize);
        }
    }
    file.close();

    m_layers.resize(m_frameOffsets.size());
    if (m_frameOffsets.empty() || !m_trajectoryStream.open(path, numberOfAtroms, &m_frameOffsets)) {
        QAbstractItemModel::endResetModel();
        clear();
        qDebug() << "[ERROR]: Failed to open xtc file!";
        return false;
    }

    emit onFramesChanged();
    qDebug() << "Loaded " << m_frameOffsets.size() << " frame offsets.";
    QAbstractItemModel::endResetModel();
    return true;
}

bool Atoms::open(const QString &modelPath, const QString &xtcPath) {
    return openModel(modelPath) && openXTC(xtcPath);
}

bool Atoms::exportLayerData(const QString &path, float sasRadius) const {
    if (m_layers.empty() || path.isEmpty()) return false;
    QFileInfo info(path);
    if (info.suffix() == "bald") {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream stream(&file);
            stream << 1111575620; //Magic number
            stream << m_model.size(); // number of atoms
            stream << m_layers.size(); // number of frames
            stream << sasRadius; // SAS radius
            for (const layerFrame &frame: m_layers) {
                stream << frame.maxLayer;
                if (frame.maxLayer != -1) {
                    QVector<unsigned char> layers;
                    layers.reserve(m_model.size());
                    for (float l: frame.layers) layers.push_back((unsigned char) l); //reduce each layer to one char
                    stream.writeRawData((char *) layers.data(), m_model.size());
                }
            }

            return true;
        }
    } else if (info.suffix() == "csv") {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            for (const atom &a: m_model)
                stream << a.name << ",";
            stream << endl;
            for (const layerFrame &frame: m_layers) {
                if (frame.maxLayer == -1) {
                    for (int i = 0; i < m_model.size(); i++)
                        stream << ",";
                } else {
                    for (float f: frame.layers)
                        stream << f << ",";
                }
                stream << endl;
            }
            return true;
        }
    }
    return false;
}

bool Atoms::importLayerData(const QString &path, float &sasRadiusOut) {
    if (m_layers.empty() || path.isEmpty()) return false;
    QFileInfo info(path);
    if (info.suffix() == "bald") {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream stream(&file);
            int magicNumber, numAtoms, numFrames;
            stream >> magicNumber; //Magic number
            stream >> numAtoms;
            stream >> numFrames;
            stream >> sasRadiusOut;

            if (magicNumber != 1111575620 || numAtoms <= 0 || numFrames <= 0 || sasRadiusOut <= 0) {
                qDebug() << "[importLayerData:" << __LINE__ << "]: Header invalid!";
                return false;
            }
            if (numAtoms != m_model.size() || numFrames != m_layers.size()) {
                qDebug() << "[importLayerData:" << __LINE__
                         << "]: Number of atoms or frames is not equal to the loaded data!" << "(" << numAtoms << "!="
                         << m_model.size() << " || " << numFrames << "!=" << m_layers.size() << ")";
                return false;
            }

            for (layerFrame &frame: m_layers) {
                stream >> frame.maxLayer;
                frame.layers.clear();
                if (frame.maxLayer != -1) {
                    QVector<unsigned char> layers(m_model.size());
                    stream.readRawData((char *) layers.data(), m_model.size());
                    frame.layers.reserve(m_model.size());
                    for (unsigned char l: layers) frame.layers.push_back((float) l);
                }
            }

            return true;
        }
    } else if (info.suffix() == "csv") {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            stream.readLine();//skipline
            auto it = m_layers.begin();
            sasRadiusOut = 1.0f;
            while (!stream.atEnd() && it != m_layers.end()) {
                QString line = stream.readLine();
                float maxLayer = -1;
                const QStringList list = line.split(QRegExp(",|;"), QString::SkipEmptyParts);
                it->layers.clear();
                if (list.size() == m_model.size()) {
                    it->layers.reserve(m_model.size());
                    for (const QString &str: list) {
                        const float v = str.toFloat();
                        it->layers.push_back(v);
                        if (v > maxLayer) maxLayer = v;
                    }
                }
                it->maxLayer = maxLayer;
                it++;
            }

            return true;
        }
    }

    return false;
}

Atoms::operator bool() const {
    return !m_model.empty() && !m_frameOffsets.empty();
}

int Atoms::numberOfAtroms() const {
    return m_model.size();
}

int Atoms::numberOfGroups() const {
    return m_groupStartIDs.size();
}

int Atoms::numberOfProteins() const {
    return m_proteinStartIDs.size();
}

int Atoms::numberOfFrames() const {
    return m_frameOffsets.size();
}

Atoms::atom &Atoms::operator[](unsigned int i) {
    return m_model[i];
}

const Atoms::atom &Atoms::operator[](unsigned int i) const {
    return m_model[i];
}

Atoms::atom &Atoms::getAtom(unsigned int i) {
    return m_model[i];
}

const Atoms::atom &Atoms::getAtom(unsigned int i) const {
    return m_model[i];
}

QVector<Atoms::atom> &Atoms::getAtoms() {
    return m_model;
}

const QVector<Atoms::atom> &Atoms::getAtoms() const {
    return m_model;
}


TrajectoryStream::xtcFrame &Atoms::getFrame(unsigned int i) {
    return m_trajectoryStream.getFrame(i);
}

TrajectoryStream::xtcFrame &Atoms::getCurrentFrame() {
    return m_trajectoryStream.getCurrentFrame();
}

const TrajectoryStream::xtcFrame &Atoms::getCurrentFrame() const {
    return m_trajectoryStream.getCurrentFrame();
}

Atoms::layerFrame &Atoms::getLayer(unsigned int i) {
    return m_layers[i];
}

const Atoms::layerFrame &Atoms::getLayer(unsigned int i) const {
    return m_layers[i];
}

QVector<Atoms::layerFrame> &Atoms::getLayers() {
    return m_layers;
}

const QVector<Atoms::layerFrame> &Atoms::getLayers() const {
    return m_layers;
}

const QString &Atoms::getHeader() const {
    return m_header;
}

const QString &Atoms::getTitle() const {
    return m_title;
}

QString Atoms::getAtomName(int index) const {
    return m_model[index].name;
}

const QVector<int> &Atoms::getGroupStartIDs() const {
    return m_groupStartIDs;
}

int Atoms::getGroupStartID(int groupindex) const {
    return m_groupStartIDs[groupindex];
}

QString Atoms::getGroupName(int index) const {
    if (index >= 0 && index < m_groupStartIDs.size())
        return m_model[m_groupStartIDs[index]].residue;
    return "";
}

int Atoms::getAtomsInGroup(int groupIndex) const {
    if (groupIndex < 0) return 0;
    if (groupIndex == m_groupStartIDs.size() - 1) {
        return numberOfAtroms() - m_groupStartIDs[groupIndex];
    } else
        return m_groupStartIDs[groupIndex + 1] - m_groupStartIDs[groupIndex];
}

float Atoms::getAtomLayer(int atomIndex, int frame, bool applyFilters, bool isTimeline) const {
    const Atoms::layerFrame &layerframe = getLayer(frame);
    if (frame < 0 || frame >= m_layers.size()) return 1.f;
    if (layerframe.maxLayer <= 0 || layerframe.layers.empty()) return -1.f;

    float value = 0;
    bool found = false;
    if (!(isTimeline && !m_filterAtomsListModel->isEnabled()) &&
        !m_filterAtomsListModel->getFilterResidueResults().isEmpty() && applyFilters) {
        for (const ResultFilterItem &v: m_filterAtomsListModel->getFilterAtomResults().value(atomIndex,
                                                                                             QList<ResultFilterItem>())) {
            if (isTimeline && !v.item->timelineViewEnabled) continue;
            if (!isTimeline && !v.item->renderViewEnabled) continue;
            if (v.item->isLive) {
                Atom::Variables vars;
                vars["frame"] = Atom::FilterSharedPtr(new Atom::Integer(frame));
                QVariant result = v.item->node->call(true, atomIndex, getAtomName(atomIndex), const_cast<Atoms *>(this),
                                                     const_cast<Atoms *>(this), m_timeline, vars);
                //qDebug()<<" RESULT "<<result<<result.type();
                if (result.type() == QVariant::Int) {
                    value += result.toInt();
                    found = true;
                } else if (result.type() == QVariant::Double) {
                    value += result.toFloat();
                    found = true;
                }
            } else {
                if (v.type() == QVariant::Int) {
                    value += v.toInt();
                    found = true;
                } else if (v.type() == QVariant::Double) {
                    value += v.toFloat();
                    found = true;
                }
            }
        }
    }
    if (found) return value;
    else return layerframe.layers[atomIndex];
}

float Atoms::getGroupLayerAvarage(int groupIndex, int frame) const {
    if (frame < 0 || frame >= m_layers.size()) {
        //qDebug()<<"["<<__LINE__<<":getGroupLayerAvarage]: frame out of bounds! ("<<frame<<", "<<m_layers.size()<<")";
        return -1;
    }
    if (groupIndex < 0 || groupIndex >= numberOfGroups())
        return -1;

    const Atoms::layerFrame &layerframe = getLayer(frame);
    if (layerframe.maxLayer > 0)
        return getGroupLayerAvarage(groupIndex, layerframe);
    return -1;
}

float Atoms::getGroupLayer(int groupIndex, int frame, bool applyFilters, bool isTimeline) const {
    float value = 0;
    bool found = false;
    if (!(isTimeline && !m_filterAtomsListModel->isEnabled()) &&
        !m_filterAtomsListModel->getFilterResidueResults().isEmpty() && applyFilters) {
        for (const ResultFilterItem &v: m_filterAtomsListModel->getFilterResidueResults().value(groupIndex,
                                                                                                QList<ResultFilterItem>())) {
            if (isTimeline && !v.item->timelineViewEnabled) continue;
            if (!isTimeline && !v.item->renderViewEnabled) continue;
            if (v.item->isLive) {
                Atom::Variables vars;
                vars["frame"] = Atom::FilterSharedPtr(new Atom::Integer(frame));
                QVariant result = v.item->node->call(false, groupIndex, getGroupName(groupIndex),
                                                     const_cast<Atoms *>(this), const_cast<Atoms *>(this), m_timeline,
                                                     vars);
                //qDebug()<<" RESULT "<<result<<result.type();
                if (result.type() == QVariant::Int) {
                    value += result.toInt();
                    found = true;
                } else if (result.type() == QVariant::Double) {
                    value += result.toFloat();
                    found = true;
                }
            } else {
                if (v.type() == QVariant::Int) {
                    value += v.toInt();
                    found = true;
                } else if (v.type() == QVariant::Double) {
                    value += v.toFloat();
                    found = true;
                }
            }
        }
    }
    if (found) return value;
    else return getGroupLayerAvarage(groupIndex, frame);
}

float Atoms::getGroupLayerAvarage(int groupIndex, const Atoms::layerFrame &frame) const {
    QVector<float>::const_iterator end = frame.layers.begin() +
                                         ((groupIndex == m_groupStartIDs.size() - 1) ? m_model.size() : m_groupStartIDs[
                                                 groupIndex + 1]);
    QVector<float>::const_iterator start = frame.layers.begin() + m_groupStartIDs[groupIndex];
    float avarage = 0;
    for (auto it = start; it < end; it++) {
        avarage += *it;
    }
    avarage /= end - start;

    //avarage = floor((avarage+0.25f)/0.5f)*0.5f;

    return avarage;
}

void Atoms::fillGroupLayerAvarage(QVector<float> &avarages, const Atoms::layerFrame &frame) const {
    avarages.fill(0.f, numberOfAtroms());

    for (int groupIndex = 0; groupIndex < m_groupStartIDs.size(); groupIndex++) {
        QVector<float>::iterator end = avarages.begin() +
                                       ((groupIndex == m_groupStartIDs.size() - 1) ? m_model.size() : m_groupStartIDs[
                                               groupIndex + 1]);

        const float avarage = (frame.maxLayer > 0) ? getGroupLayerAvarage(groupIndex, frame) : 0;
        for (auto it = avarages.begin() + m_groupStartIDs[groupIndex]; it < end; it++) {
            *it = avarage;
        }
    }
}

void Atoms::fillGroupLayer(QVector<float> &avarages, int frame, bool applyFilters, bool isTimeline) const {
    avarages.fill(0.f, numberOfAtroms());

    for (int groupIndex = 0; groupIndex < m_groupStartIDs.size(); groupIndex++) {
        QVector<float>::iterator end = avarages.begin() +
                                       ((groupIndex == m_groupStartIDs.size() - 1) ? m_model.size() : m_groupStartIDs[
                                               groupIndex + 1]);

        const float value = getGroupLayer(groupIndex, frame, applyFilters, isTimeline);
        for (auto it = avarages.begin() + m_groupStartIDs[groupIndex]; it < end; it++) {
            *it = value;
        }
    }
}

void Atoms::fillAtomLayer(QVector<float> &atomLayers, int frame, bool applyFilters, bool isTimeline) const {
    if (!applyFilters) {
        atomLayers = getLayer(frame).layers;
        return;
    }
    atomLayers.fill(0.f, numberOfAtroms());
    for (int atomIndex = 0; atomIndex < numberOfAtroms(); atomIndex++) {
        atomLayers[atomIndex] = getAtomLayer(atomIndex, frame, applyFilters, isTimeline);
    }
}


QVector<Atoms::atom>::iterator Atoms::findAtomInGroup(int groupIndex, const QString &atomName) {
    if (groupIndex < 0 || groupIndex >= m_groupStartIDs.size()) return m_model.end();
    QVector<Atoms::atom>::iterator end = m_model.begin() +
                                         ((groupIndex == m_groupStartIDs.size() - 1) ? m_model.size() : m_groupStartIDs[
                                                 groupIndex + 1]);

    for (auto it = m_model.begin() + m_groupStartIDs[groupIndex]; it < end; it++)
        if (it->name == atomName) return it;

    return m_model.end();
}

QVector<Atoms::atom>::const_iterator Atoms::findAtomInGroup(int groupIndex, const QString &atomName) const {
    if (groupIndex < 0 || groupIndex >= m_groupStartIDs.size()) return m_model.end();
    QVector<Atoms::atom>::const_iterator end = m_model.begin() +
                                               ((groupIndex == m_groupStartIDs.size() - 1) ? m_model.size()
                                                                                           : m_groupStartIDs[
                                                        groupIndex + 1]);

    for (auto it = m_model.begin() + m_groupStartIDs[groupIndex]; it < end; it++)
        if (it->name == atomName) return it;

    return m_model.end();
}

const QVector<int> &Atoms::getProteinStartIDs() const {
    return m_proteinStartIDs;
}

QString Atoms::getFullResidueName(const QString &residue) const {
    return m_residuesFullName.value(residue, "Undefined");
}

bool Atoms::createBond(int fromResidue, const QString &fromAtomName, int toResidue, const QString &toAtomName) {
    auto itFrom = findAtomInGroup(fromResidue, fromAtomName);
    if (itFrom == m_model.end()) return false;
    auto itTo = findAtomInGroup(toResidue, toAtomName);
    if (itTo == m_model.end()) return false;
    if (itFrom->proteinID != itTo->proteinID) return false;

    m_bonds.push_back({(int) (itFrom - m_model.begin()), (int) (itTo - m_model.begin())});

    return true;
}

const QVector<Atoms::bund> &Atoms::getBonds() const {
    return m_bonds;
}

int Atoms::getHoveredAtom() const {
    return m_hoveredAtom;
}

void Atoms::setHoveredAtom(int id) {
    if (m_hoveredAtom != id) {
        m_hoveredAtom = id;
        if (m_hoveredAtom >= 0 && m_hoveredAtom < numberOfAtroms())
            m_hoveredGroup = m_model[m_hoveredAtom].groupID;
        else
            m_hoveredGroup = -1;
        emit hoveredChanged();
    }
}

int Atoms::getHoveredGroup() const {
    return m_hoveredGroup;
}

void Atoms::setHoveredGroup(int id) {
    if (m_hoveredGroup != id) {
        m_hoveredGroup = id;
        m_hoveredAtom = -1;
        emit hoveredChanged();
    }
}

void Atoms::setHovered(int atomID, int groupID) {
    if (atomID != m_hoveredAtom || groupID != m_hoveredGroup) {
        m_hoveredAtom = atomID;
        m_hoveredGroup = groupID;
        emit hoveredChanged();
    }
}


void Atoms::setSelectedAtom(int id) {
    if (id >= m_model.size()) id = -1;

    if (m_selectedAtom != id) {
        m_selectedAtom = id;
        emit selectionChanged();
    }
}

void Atoms::setSelectedGroup(int id) {
    if (id < 0) {
        setSelectedAtom(-1);
    } else
        setSelectedAtom(getGroupStartID(id));
}

int Atoms::getSelectedAtom() const {
    return m_selectedAtom;
}

int Atoms::getSelectedGroup() const {
    if (m_selectedAtom >= 0)
        return m_model[m_selectedAtom].groupID - 1;
    return -1;
}


void Atoms::clear() {
    if (m_filterAtomsListModel) m_filterAtomsListModel->clear();
    m_header.clear();
    m_title.clear();
    m_model.clear();
    m_frameOffsets.clear();
    m_layers.clear();
    m_bonds.clear();
    m_groupStartIDs.clear();
    m_proteinStartIDs.clear();
    m_trajectoryStream.clear();
    m_waterCount = 0;

    //selection
    m_hoveredAtom = -1;
    m_hoveredGroup = -1;
    m_selectedAtom = -1;
    m_selectedGroup = -1;
    emit onModelDataChanged();
}

QVariant Atoms::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.parent().isValid()) {
        switch (role) {
            case Qt::DisplayRole: {
                int atomIndex = getGroupStartID(index.parent().row()) + index.row();
                return QVariant::fromValue(getAtomName(atomIndex));
            }
                break; //the display text
            case TypeRole: {
                return QVariant::fromValue(QString("atom"));
            }
                break;
            case isAtomRole: {
                return QVariant::fromValue(true);
            }
                break;
            case ItemIndexRole: {
                return QVariant::fromValue(index);
            }
                break;
            case ItemRowRole: {
                int atomIndex = getGroupStartID(index.parent().row()) + index.row();
                return QVariant::fromValue(atomIndex);
            }
                break;
        }
    } else {
        switch (role) {
            case Qt::DisplayRole: {
                return QVariant::fromValue(getGroupName(index.row()));
            }
                break; //the display text
            case TypeRole: {
                return QVariant::fromValue(QString("residue"));
            }
                break;
            case isAtomRole: {
                return QVariant::fromValue(false);
            }
                break;
            case ItemIndexRole: {
                return QVariant::fromValue(index);
            }
                break;
            case ItemRowRole: {
                return QVariant::fromValue(index.row());
            }
                break;
        }
    }


    return QVariant();
}

Qt::ItemFlags Atoms::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant Atoms::headerData(int, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant::fromValue(QString("HEADER"));

    return QVariant();
}

QModelIndex Atoms::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid()) {
        return createIndex(row, column, (int) parent.row());
    }
    return createIndex(row, column, (int) -1);
}

QModelIndex Atoms::parent(const QModelIndex &index) const {
    if (!index.isValid() || (int) index.internalId() < 0) {
        return QModelIndex();
    }

    //QModelIndex();
    return createIndex(index.internalId(), 0, (int) -1);
}

int Atoms::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        if (parent.parent().isValid())
            return 0;
        return getAtomsInGroup(parent.row());
    }
    return numberOfGroups();
}

int Atoms::columnCount(const QModelIndex &) const {
    return 1;
}

QHash<int, QByteArray> Atoms::roleNames() const {
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[TypeRole] = "type";
    roles[isAtomRole] = "isAtom";
    roles[ItemIndexRole] = "itemIndex";
    roles[ItemRowRole] = "itemRow";
    return roles;
}


Atoms::~Atoms() {

}

QDebug operator<<(QDebug d, const glm::vec2 &m) {
    d << "(" << m.x << ", " << m.y << ")";
    return d;
}

QDebug operator<<(QDebug d, const glm::vec3 &m) {
    d << "(" << m.x << ", " << m.y << ", " << m.z << ")";
    return d;
}

QDebug operator<<(QDebug d, const Atoms::atom &m) {
    d << "Atom[typeName: " << m.name << " atomName: " << m.element << " groupName: " << m.residue << " groupID: "
      << m.groupID << " position: " << m.position << "]";
    return d;
}

QDebug operator<<(QDebug d, const TrajectoryStream::xtcFrame &m) {
    d << "Frame[time: " << m.time << " precision: " << m.precision << " size: " << m.positions.size() << " i: "
      << m.index << "]";
    return d;
}

QDebug operator<<(QDebug d, const Atoms &m) {
    d << "Atom[numberOfAtroms: " << m.numberOfAtroms() << " numberOfFrames: " << m.numberOfFrames() << "]";
    return d;
}

