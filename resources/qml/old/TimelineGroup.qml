import QtQuick 2.0

Item{
	property int startingAtomIndex: 0
	property int atomSize: 0
	property int itemHeight: 20
	property bool open: false
	
	height: itemHeight + ((open)? 15*atomSize+10: 0); 

	width:root.width;
	Column {
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
				source: ("image://imageprovider/group-"+model.index);
				//asynchronous: true; //fast but looks ugly
				
				Connections {
					target: mainWiget
					onUpdateHeatMap: { heatmap.source = ""; heatmap.source = ("image://imageprovider/group-"+model.index);}
				}
			}
			Rectangle {
				id: textBox
				x: (root.hideText)? -100: 0;
				Text{
					id: itemName
					color: colorLib.light
					text: ((open)?  "\u25BE " : "\u25B8 ") + (1+model.index)+":"+ model.display; //(1+groupIndex)+":"+atomsData.getGroupName(groupIndex);
					font.pointSize: 12
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
				border.width: ((atomsData.hoveredGroup == model.index+1)? 1: 0);
				
				MouseArea {
					
					hoverEnabled: true;
					anchors.fill: parent;
					acceptedButtons: Qt.LeftButton | Qt.RightButton; //Qt.AllButtons;
					onClicked:{
						open = !open;
					}
					onEntered:{
						atomsData.hoveredGroup  = model.index+1;
					}
					onExited:{
						atomsData.hoveredGroup = -1;
						infobox.x = -100;
						infobox.y = -100;
					}
					onPositionChanged: {
						root.hideText = mouse.x < 200;
						
						var coords = this.mapToItem(root, mouse.x, mouse.y);
						var frame = Math.floor(timeline.start + (coords.x/ruler.width) * (timeline.end-timeline.start));
						var avarage = atomsData.getGroupLayerAvarage(model.index, frame);
						if(avarage != -1){
							infobox.layerVal = avarage;
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
		
		//atomsData
		Column {
			Rectangle {
				width: parent.width
				height: ((open)? 5: 0);
				color: colorLib.window
			}
			Repeater {
				model: ((open)? atomSize: 0);
				delegate: TimelineAtom{
					atomIndex: startingAtomIndex+index;
					height: 15;
				}
			}
			Rectangle {
				width: parent.width
				height: ((open)? 5: 0);
				color: colorLib.window
			}
		}
	}
}