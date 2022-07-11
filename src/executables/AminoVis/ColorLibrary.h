/*!
 * @file   		ColorLibrary.h
 * @author 		Vladimir Ageev (vladimir.agueev@progsys.de)
 * @date   		13.06.2017
 *
 * @brief Manages heatmap color interpolation and window/qml style.
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
#ifndef EXECUTABLES_AMINOVIS_COLORLIBRARY_H_
#define EXECUTABLES_AMINOVIS_COLORLIBRARY_H_

#include <QObject>
#include <QColor>
#include <QVector>
#include <QSettings>
#include <QDataStream>
#include <QAbstractTableModel>
#include <QPalette>
#include <QBrush>

typedef union {
    struct {
    	unsigned char b;
    	unsigned char g;
    	unsigned char r;
    	unsigned char a;
    } bgra;
    int data;
} rawcolor;

rawcolor operator*(rawcolor c, float f);
rawcolor operator+(rawcolor a, rawcolor b);
rawcolor interpolate(rawcolor a, rawcolor b, float val);

QDataStream& operator<< (QDataStream& s, const QVector<rawcolor>& data);
QDataStream& operator>> (QDataStream& s, QVector<rawcolor>& data);
QDataStream& operator<< (QDataStream& s, const rawcolor& data);
QDataStream& operator>> (QDataStream& s, rawcolor& data);

Q_DECLARE_METATYPE(QVector<rawcolor>)

/*!
 * @brief A table model for color interpolation.
 *
 * For easer edit the colors that are used for interpolation are defined as an QAbstractTableModel.
 * This makes it possible to use this class as a model for a QTableView and implement useful functions like Click & Drop.
 *
 * @see http://doc.qt.io/qt-4.8/modelview.html
 */
class ColorsRow : public QAbstractTableModel {
	Q_OBJECT
public:
	ColorsRow(QObject *parent = 0);
	ColorsRow(const QVector<rawcolor> &v, QObject *parent = 0);
	virtual ~ColorsRow();

	void operator=(const ColorsRow& colors);
	void operator=(const QVector<rawcolor> &v);

	QColor getColor(int index) const;

	QColor getInterpolatedColor(float val) const;
	rawcolor getInterpolatedRawColor(float val) const;

	void setColor(int index, const QColor& colorIn);
	void setColor(int index, const rawcolor& colorIn);


	void push_back(const QColor& colorIn);
	void moveLeft(int index);
	void moveRight(int index);
	void remove(int index);

	inline rawcolor& operator[](int i) {return m_colors[i];}
	inline const rawcolor& operator[](int i) const{return m_colors[i];}
	inline bool empty() const{return m_colors.empty();}
	inline int size() const{return m_colors.size();}
	inline rawcolor& last(){return m_colors.last();}
	inline const rawcolor& last() const{return m_colors.last();}
	inline rawcolor& front(){return m_colors.front();}
	inline const rawcolor& front() const{return m_colors.front();}

	inline QVector<rawcolor>& vec(){return m_colors;}
	inline const QVector<rawcolor>& vec() const{return m_colors;}

	inline const void* ptr() const{return m_colors.data();}
	inline QVector<rawcolor>::iterator begin(){return m_colors.begin();}
	inline QVector<rawcolor>::const_iterator begin() const{return m_colors.begin();}
	inline QVector<rawcolor>::iterator end(){return m_colors.end();}
	inline QVector<rawcolor>::const_iterator end() const{return m_colors.end();}

	virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const Q_DECL_OVERRIDE;
	virtual QMimeData* mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE;
	virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
signals:
	void colorChanged();
private:
	QVector<rawcolor> m_colors;
};

/*!
 * @brief  Manages heatmap color interpolation and window/qml style colors.
 *
 * This class is used to dynamically share the different color settings between the .qcc style file, QML and the custom GL rendering.
 * Additionally this manages the layer color interpolation for the heatmap and the atoms inside the GL rendering.
 * All the data is then stored and loaded from a config file.
 * @see ColorsRow
 */
class ColorLibrary: public QObject  {
	Q_OBJECT
	//using my own color defines as 'QApplication::setPalette(palette());' requires a file reload
	Q_PROPERTY(QColor alternateBase READ getAlternateBase NOTIFY onStyleChanged)
	Q_PROPERTY(QColor base READ getBase NOTIFY onStyleChanged)
	Q_PROPERTY(QColor highlight READ getHighlight NOTIFY onStyleChanged)
	Q_PROPERTY(QColor light READ getLight NOTIFY onStyleChanged)
	Q_PROPERTY(QColor mid READ getMid NOTIFY onStyleChanged)
	Q_PROPERTY(QColor text READ getText NOTIFY onStyleChanged)
	Q_PROPERTY(QColor window READ getWindow NOTIFY onStyleChanged)
	Q_PROPERTY(QColor windowTransparent READ getWindowTransparent NOTIFY onStyleChanged)
	Q_PROPERTY(QColor windowText READ getWindowText NOTIFY onStyleChanged)
public:
	ColorsRow p_layersColors; ///Colors for each SAS layer
	ColorsRow p_glLayersColors; ///If enabled then these layer colors will be used for the gl view

	/*!
	 * @param file The name of the config file to be used to load and save color presets.
	 */
	ColorLibrary(const QString& file = "colors.ini", QObject* parent = nullptr);
	virtual ~ColorLibrary();

	/*!
	 * @returns The currently used color preset name.
	 */
	const QString& getPreset() const;
	/*!
	 * @returns A list with names of all available color presets.
	 */
	inline QStringList getPresets() const{
		return m_colorSettings.childGroups();
	}
	/*!
	 * @brief Deletes a given preset.
	 * @param name The name of the preset.
	 */
	void removePreset(const QString& name);

	/*!
	 * @returns True, if the gl rendering should use different colors.
	 */
	bool getOverwriteGLLayerColors() const;
	/*!
	 * @brief Gives you the color row that is used to get an interpolated color of a layer value.
	 * Depending on the given boolean and if the gl rendering should use different colors, this method will return
	 * the appropriate color row defined by the color preset.
	 * @param heatmap Should be true if the wanted color row is for the heatmap.
	 * @returns ColorsRow
	 */
	const ColorsRow& getLayerColors(bool heatmap = true) const;

	/*!
	 * @brief Should the default gl background color be overwritten by a custom color.
	 * @returns True if gl background color should be overwritten
	 */
	bool getOverwriteGLBackground() const;
	/*!
	 * @returns Background color that the gl rendering should use.
	 */
	const QColor& getGLBackgroundColor() const;

	QPalette& palette();
	const QPalette& palette() const;

	const QColor& getAlternateBase() const;
	const QColor& getBase() const;
	const QColor& getHighlight() const;
	const QColor& getLight() const;
	const QColor& getMid() const;
	const QColor& getText() const;
	const QColor& getWindow() const;
	const QColor getWindowTransparent() const;
	const QColor& getWindowText() const;

public slots:
	/*!
	 * @brief Set the color preset that should be used by the application.
	 * This methods checks if the given color present already exists inside the config file.
	 * If it does then it gets loaded, otherwise it get defined with the colors of the previous used color preset.
	 * @param preset The name of the preset.
	 */
	void setPreset(const QString& preset);
	void setPalette(const QPalette& p);

	void setOverwriteGLLayerColors(bool enable);
	void setOverwriteGLBackground(bool enable);
	void setGLBackgroundColor(const QColor& color);

	/*!
	 * @brief Loads the color preset settings from the config file.
	 */
	void load();
	void load(const QString& preset);
	/*!
	 * @brief Saves the current color preset settings into the config file.
	 */
	void save();
	void save(const QString& preset);
signals:
	void colorsChanged();
	void onStyleChanged();
private:
	QString m_preset = "Default";
	bool m_overwriteGLLayerColors = false;
	bool m_overwriteGLBackground = false;

	QColor m_glBackground;

	QSettings m_colorSettings;
	QPalette m_palette;
};



#endif /* EXECUTABLES_AMINOVIS_COLORLIBRARY_H_ */
