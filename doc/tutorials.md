Tutorials {#tuto}
=========

@section tuto_create_from_scratch Create an aircraft from scratch

In this tutorial, we will present how to create an aircraft from scratch. 

1 ) Open CPACSCreator.     
2 ) Create a new file, either click on create a new file (File->New) or press the shortcut Ctl+N.

@image html images/tutorial_scratch/tuto_scratch_2.png    
\n

3 ) In the dialog box, choose one of the template files, then click on "OK".  

@image html images/tutorial_scratch/tuto_scratch_3.png            
\n

4 ) Select the fuselages element in the CPACS tree view. 
If the CPACS tree view is not shown either activate the view trough the menu (View->Display->Tree) 
or press the shortcut alt+M.    
  
@image html images/tutorial_scratch/tuto_scratch_4.png            
\n

5 ) Click on "Add a new fuselage" in the Editor widget.
  If the editor widget is not shown, either activate the widget through the menu (View->Display->Editor)
  or enter the shortcut alt+Shift+M.
  
@image html images/tutorial_scratch/tuto_scratch_5.png            
\n

6 ) In the dialog box, choose the UID for this fuselage, the number of section and the profile UIDs, then click on "OK".
It is important to give the correct UID at this point, because, for the moment,
it is impossible to change UID later on. Observe, that the other UIDs such as section UIDs or the transformation UIDs
are automatically generated and derived from this given UID.
   
@image html images/tutorial_scratch/tuto_scratch_6.png            
\n

7 ) Click on the button to fit the content on the view port or enter the shortcut Alt+A,
because you are probably to far away from the fuselage.

@image html images/tutorial_scratch/tuto_scratch_7.png            
\n

8 ) In the CPACS tree view, click on the fuselage that you have just created (here "MainFuselage") to activate its editor widget.

@image html images/tutorial_scratch/tuto_scratch_8.png            
\n
     
9 ) Now you should have the editor widget displaying the high level parameters for the fuselage 
 and the fuselage sections highlighted in the scene. 
  
@image html images/tutorial_scratch/tuto_scratch_9.png            
\n

10 ) Edit some parameters of the fuselage. Here, I change the length to 15 and the width to 3, then click on "OK".
Remark, that the change are only applied when you click on "OK". The apply order is from top parameter to bottom parameter.
If you have made a mistake and you want the values of your current tigl object to be restored, you can click on "Cancel".
   
@image html images/tutorial_scratch/tuto_scratch_10.png            
\n

11 ) Now, if you want to go in a more fine-grained definition, you need to switch to the section level. To do so, select the wanted 
section in the CPACS tree view and click on it. The editor widget should change to the section editor and only the
selected section should be highlighted. Here, I chose the first section.

@image html images/tutorial_scratch/tuto_scratch_11.png            
\n


12 ) Edit the high-level parameters of the section. Here, I changed the area to zero to have a peaked nose.

@image html images/tutorial_scratch/tuto_scratch_12.png            
\n

13 ) Do the same for the other sections, if you want. 
To set the position of a section you can also use the positioning editor. Remark the difference between setting the 
position using the section editor and setting the position using the positioning editor. 
In the case of using the section editor, only the selected section changed and in the case of using the positioning
widget, the section after the selected section was shifted. This comes from the fact that the positionings depend
on each other. 


@image html images/tutorial_scratch/tuto_scratch_13a.png "Set the position of section 2 using center in section widget"
\n


@image html images/tutorial_scratch/tuto_scratch_13b.png  "Set the position of section 2 using positioning in positioning widget"          
\n

14 ) If you want to add a section, you can go to the sections level by click on "sections" in the tree view and then 
click on "Add section". Here, I add a section after the last one. 
Remark that the position and the size of the added section is defined by the other sections already present in the fuselage.
 
@image html images/tutorial_scratch/tuto_scratch_14a.png "Sections level"
\n


@image html images/tutorial_scratch/tuto_scratch_14b.png  "Add section dialog box"          
\n     
 
@image html images/tutorial_scratch/tuto_scratch_14c.png  "Once the section was added"          
\n     
    

15 ) Modify the fuselage as you want by the selecting the object to modify in the tree view and edit in the
editor widget. If you have made some unwanted operation you can always undo the previous operation by 
pressing the shortcut Ctl+Z if the focus is in the scene or by clicking on the undo button (Edit->Undo)  
 
@image html images/tutorial_scratch/tuto_scratch_15.png         
\n     
    
16 ) Once you are happy with your fuselage or when you want to add a check point, you can save your file. Click on 
(File->Save) or press the short Ctl+S. Make sure to add .xml to the file name otherwise an error will rise. 
Remark, before this operation the file is not saved. There is no auto-saving feature. 

@image html images/tutorial_scratch/tuto_scratch_16.png  "Once the section was added"          
\n     

17 ) The creation of the wing is basically the same as the creation of a fuselage. First, add a new wing.

@image html images/tutorial_scratch/tuto_scratch_17.png         
\n     


18 ) Edit the wing parameters. 

@image html images/tutorial_scratch/tuto_scratch_18.png   
\n     
    
19 ) Go in more fine-grained definition.

 @image html images/tutorial_scratch/tuto_scratch_19.png    
 \n     
     
20 ) You can of course create multiple wings.

@image html images/tutorial_scratch/tuto_scratch_20.png    
 \n 
 
21 ) If you want to create a vertical wing, you can simply set the X-rotation to 90 degrees.
Remark that the sweep, dihedral and other definitions still make also sense in this case. 

@image html images/tutorial_scratch/tuto_scratch_21.png
\n

22 ) You can chang the transparency of the displayed element by selecting the element and right clicking on them.

@image html images/tutorial_scratch/tuto_scratch_22.png
\n    
    
23 ) You can also export your geometry to other formats.     
      
@image html images/tutorial_scratch/tuto_scratch_23.png
\n    

24 ) :)


@section tuto_python Python Scripting 

See the section @ref lib_intrernal_python_demo
and take a look at the example file create_space_ship.py that explains the mechanism of the internal python api quite well.
