import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import Frame 0.1

Rectangle { 
	id: root
	color: colorLib.window
	property bool hideText: false
	 
	//top
	Item{
		id: rulerItem
		anchors.top: parent.top
		height: ruler.height
		Ruler{
			id: ruler
			width: root.width-20
			height: 21
			stepSize: 40
			start: timeline.startVis
			end: timeline.endVis
		}
		
		MouseArea {
			width: ruler.width
			height: ruler.height
			preventStealing: false;
			onPositionChanged:{
				timeline.getR(timeline.activeTracker).frame =  Math.floor(timeline.start + (mouse.x/ruler.width) * (timeline.end-timeline.start))
				root.hideText = false;
				markerLine.x = trackerbox.x+2;
				mouse.accepted = true;
			}
			onReleased:{
				markerLine.x = -100;
			}
			onClicked:{
				timeline.getR(timeline.activeTracker).frame =  Math.floor(timeline.start + (mouse.x/ruler.width) * (timeline.end-timeline.start))
			}
		}
		//active tracker
		Rectangle {
			id: trackerbox
			anchors.top: parent.top
			width: 5
			height: 21
			color: timeline.getR(timeline.activeTracker).color
			x: -2 
		}

		Binding {
			target: trackerbox
			property: "x"
			value: ((timeline.getR(timeline.activeTracker).frame-timeline.start)/(timeline.end-timeline.start))*ruler.width -2
			when: timeline.getR(timeline.activeTracker).onFrameChanged
		}
		
		
		//trackers at the background
		Repeater {
			model: timeline.size
			Item{
				visible: index != timeline.activeTracker
				Rectangle {
					id: backtrackerbox
					//anchors.bottom: rulerItem.bottom
					y: 10
					width: 5
					height: 15//21
					color: timeline.getR(index).colorGray
					x: -2 
					
					MouseArea {
						anchors.fill: parent;
						acceptedButtons: Qt.LeftButton ;
						hoverEnabled: true;
						preventStealing: false;
						cursorShape: Qt.SizeHorCursor;
						//propagateComposedEvents: true;
						onClicked:{
							timeline.activeTracker = index;
						}
						onPressed:{
							timeline.activeTracker = index;
							mouse.accepted = false;
						}
						onEntered:{
							parent.color = timeline.getR(index).color;
						}
						onExited:{
							parent.color = timeline.getR(index).colorGray;
						}
					}
				}
			
				Binding {
					target: backtrackerbox
					property: "x"
					value: ((timeline.getR(index).frame-timeline.start)/(timeline.end-timeline.start))*ruler.width -2
					when: timeline.getR(index).onFrameChanged
				}
			}
		}
		//timeline tracker position number
		Rectangle {
			color: colorLib.windowTransparent
			anchors.bottom: trackerbox.bottom
			anchors.left: trackerbox.right
			anchors.leftMargin: 1
			anchors.topMargin: 0
				Text { 
					color: colorLib.text
					text: timeline.getR(timeline.activeTracker).value
					font.pointSize: 7.8
				}
			width: childrenRect.width
			height: childrenRect.height
		}
	}
	
	
	TreeView { 
		id: dataView
		anchors.topMargin: ruler.height;
		anchors.fill: parent
		horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
		verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
		backgroundVisible: false
		
		style: TreeViewStyle {
			branchDelegate:Text{
					color: colorLib.light
					text: styleData.isExpanded ? "\u25BE " : "\u25B8 "
				}
			frame: Rectangle {
				color:  colorLib.window
			}
			
			/*Rectangle {
				width: 15; height: 15
				color: styleData.isExpanded ? "red" : "green"
			}*/
		}
	
		TableViewColumn {  
		   width: 100  
		   title: "Name"  
		   role: "display"  
		}  
		headerVisible:false
		model: atomsFiltered
		
		itemDelegate: Rectangle {
				id: textBox
				//anchors {left: parent.left; right: parent.right; }  
				//x: (root.hideText)? -100: 0;
				visible: !root.hideText
				Text{
					id: itemName
					color: colorLib.light
					text: (model)? (1+model.itemRow)+":"+ model.display : ""; //(1+groupIndex)+":"+atomsData.getGroupName(groupIndex);
					font.pointSize: (model && model.isAtom)? 10: 12 
					//font.bold: true
				}
				color: "#800F0F0F"
				height: 25
				width: itemName.paintedWidth;
			}

		
		rowDelegate:  Item {
						height: 25 
					Loader {
						id: loader
						//height: 30 //(parent.model.isAtom)? 20: 30  
						property var itemModel: (typeof model !== "undefined")? model: null
						property var currentItemIndex: styleData.row 
						sourceComponent: (itemModel)? (itemModel.isAtom)? atomDelegate: residueDelegate : null
						
						//onStatusChanged: if (loader.status == Loader.Ready) console.log('Loaded')
					}
				}
		
		AtomDelegate{ id: atomDelegate }
		ResidueDelegate{ id: residueDelegate }
	}
	
	//function positionViewAtRow(row, mode) { __listView.positionViewAtIndex(row, mode) }
	function scrollToRow(row){
		
		var qIndex = atomsFiltered.mapFromSource(row)
		console.log(row,qIndex)
        dataView.__listView.positionViewAtIndex(qIndex.row, ListView.Beginning);
    }
	
	Connections {
		target: mainWidget
		function onTimelineScrollTo(row) {
			console.log(row)
			scrollToRow(row)
		}
	}
	
	function isExpanded(row) {
		return dataView.isExpanded(row);
	}
	
	/*
	ScrollView {
		anchors.topMargin: ruler.height;
		anchors.fill: parent
		horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
		verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
		

			
			
		
		TableView { 
			width: 180; height: 200
           //anchors.fill: parent  
           model: atomsData //atomsFiltered  
		   headerVisible:false
           itemDelegate: Rectangle {  
               color: ( styleData.row % 2 == 0 ) ? "white" : "lightblue"  
               //anchors {left: parent.left; right: parent.right; }  
               height: 20  
      
               Text {  
                   anchors.verticalCenter: parent.verticalCenter  
                   anchors.left: parent.left  
                   text: model.index+":"+model.display //styleData.value // this points to the role we defined in the TableViewColumn below; which one depends on which column this delegate is instantiated for.  
               }  
           }  
      
           TableViewColumn {  
               width: 100  
               title: "Name"  
               role: "display"  
           }  
       }  

		ListView {
		//width: 180; height: 200
		model: atomsFiltered
				delegate: Rectangle{
					width:100
					height:20
					Text { 
						color: colorLib.text
						text: model.index+":"+model.display
						font.pointSize: 7.8
					}
					color:"#00B000"
				}
		}
		
	}*/
		
	Rectangle {
		property real layerVal: 0
		id: infobox
		x: -100
		y: -100
		height: 27
		color: colorLib.base
		border.color: "black"
		border.width: 1
		Text{
			id: infoboxText
			x: 5
			y: 4
			color: colorLib.windowText
			text: Number(parent.layerVal).toFixed(3);
			font.pointSize: 12
		}
		width: infoboxText.paintedWidth+10
	}
	
	Rectangle {
		id: markerLine
		color: timeline.getR(timeline.activeTracker).color
		x: -100
		y:21
		height: root.height
		width: 1
	}
		
	MouseArea {
		anchors.fill: parent;
		anchors.topMargin: ruler.height;
		acceptedButtons: Qt.MiddleButton;
		propagateComposedEvents: true
		preventStealing: false
		property var lastX: 0
		
		onPositionChanged: {
			//drag move
			if(lastX > 0){
				var tdif = timeline.end-timeline.start;
				var dif = (mouse.x-lastX)*(tdif/ruler.width);
				if(timeline.start-dif < 0){
					timeline.start = 0;
					timeline.end = tdif;
				}else if(timeline.end-dif > timeline.max){
					timeline.end = timeline.max;
					timeline.start = timeline.max-tdif;
				}else{
					timeline.start-=dif;
					timeline.end-=dif;
				}
				lastX = mouse.x;
			}
		}
		
		onExited: {
			root.hideText = false;
		}
		
		onPressed: {
			//console.log(mouse.x);
			cursorShape = Qt.ClosedHandCursor;
			this.lastX = mouse.x;
		}
		
		onReleased: {
			//console.log("onReleased", mouse.buttons);
			cursorShape = Qt.ArrowCursor;
			this.lastX = -1;
		}
		
		onWheel: {
			if (wheel.modifiers & Qt.ControlModifier) {
				var scale = (70 * (1.0-((timeline.end-timeline.start)/timeline.max)))+1;
				//console.log("x ", wheel.x, "y ", wheel.y, "a ", wheel.angleDelta.y, "s ", scale, ((timeline.end-timeline.start)/timeline.max), (wheel.angleDelta.y/scale));
				timeline.start += (wheel.angleDelta.y/scale)*80*(wheel.x/ruler.width);
				timeline.end -= (wheel.angleDelta.y/scale)*80*(1-wheel.x/ruler.width);
				if(timeline.start == timeline.end) timeline.end++;
			}else wheel.accepted = false;
		}
	}
}
