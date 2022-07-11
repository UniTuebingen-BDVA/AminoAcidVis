import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
	property int start: 0
	property int end: 1000
	property int stepSize: 10

	id: ruler
	color: colorLib.base

	enabled: false
	height: 24
	
	Repeater {
		model: Math.round(ruler.width/stepSize) 
		Rectangle {
			anchors.top: ruler.top
			height: ruler.height //(index % 4)? ((index % 2) ? 3 : 7) : 25
			width: 1
			color: colorLib.windowText
			x: index * ruler.width/Math.round(ruler.width/stepSize) 
		}
	}

	Repeater {
		model: Math.round(ruler.width/stepSize) 
		Label {
			anchors.top: ruler.top
			anchors.topMargin: -2
			color: colorLib.windowText
			x: index * ruler.width/Math.round(ruler.width/stepSize) + 3
			text: Math.floor(start + index * (end-start)/Math.round(ruler.width/stepSize))
			font.pointSize: 7.5
		}
	}
}
