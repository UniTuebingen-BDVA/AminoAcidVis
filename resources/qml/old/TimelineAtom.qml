import QtQuick 2.0

Item{

	property int atomIndex: -1
	height: 20; 

	width:ruler.width;

	Item{
		height:itemHeight;
		width:parent.width;
		Image{
			id: heatmap
			width: parent.width;
			height: parent.height;
			cache: false;
			sourceSize.width: parent.width;
			sourceSize.height: parent.height;
			source: ("image://imageprovider/atom-"+atomIndex);
			//asynchronous: true;
			
			Connections {
				target: mainWiget
				onUpdateHeatMap: { heatmap.source = ""; heatmap.source = ("image://imageprovider/atom-"+atomIndex);}
			}
		}
		Rectangle {
			id: textBox
			x: (root.hideText)? -100: 0;
			Text{
				id: itemName
				color: colorLib.light
				text: "   "+(1+atomIndex)+":"+atomsData.getAtomName(atomIndex);
				font.pointSize: 10
				//font.bold: true
			}
			color: "#800F0F0F"
			width: itemName.paintedWidth;
			height: itemName.paintedHeight;	
		}
		Rectangle {
			color: "transparent"
			anchors.fill: parent;
			border.color: colorLib.highlight;
			border.width: ((atomsData.hoveredAtom == atomIndex)?1:0);
			
			MouseArea {
				
				hoverEnabled: true;
				anchors.fill: parent;
				acceptedButtons: Qt.LeftButton | Qt.RightButton; //Qt.AllButtons;
				onClicked:{
					//TODO
				}
				onEntered:{
					atomsData.hoveredAtom = atomIndex;
				}
				onExited:{
					atomsData.hoveredAtom = -1;
					
					infobox.x = -100;
					infobox.y = -100;
				}
				onPositionChanged: {
					root.hideText = mouse.x < 200;
						
					var coords = this.mapToItem(root, mouse.x, mouse.y);
					var frame = Math.floor(timeline.start + (coords.x/ruler.width) * (timeline.end-timeline.start));
					var atomlayer = atomsData.getAtomLayer(atomIndex, frame);
					if(atomlayer != -1){
						infobox.layerVal = atomlayer;
						infobox.x = (coords.x+10 > root.width-infobox.width)? coords.x-infobox.width :coords.x+5;
						infobox.y = coords.y+10;
					}else{
						infobox.x = -100;
						infobox.y = -100;
					}
				}
			}
		}
	}
}