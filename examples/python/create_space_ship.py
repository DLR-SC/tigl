import tixi3
import tixi3.tixi3wrapper
import tigl3
import tigl3.tigl3wrapper
import tigl3.configuration
import tigl3.geometry
import OCC.gp


def create_fuselage(aircraft, uid):
    
    # Get the fuselageS object
    fuselages = aircraft.get_fuselages()
    # fuselageS object is of the type CCPACSFuselages,
    # so it has a function to create a new fuselage.
    numberOfSection = 22
    fuselage = fuselages.create_fuselage(uid, numberOfSection, "fuselageCircleProfileuID")
    
    
    # The created fuselage has 15 sections and has a cylindric shape.
    # We first set heigh level parameters of the fuselage to get the wanted global width and height
      
    fuselage.set_max_height(0.7)
    fuselage.set_max_width(0.7)
    
    # Now we want to give a more interesting shape to this fuselage.
    # There is multiple way to achieve this. 
    # But here we chose to use the handy CTiglSectionElement class. 
    # This class is able to set the center, the height, the width, the area,
    # the normal, the rotation around the normal of each section in global or parent coordinate system. 
    # To access a instance of this class, it is a little bit tricky. 
    # You first need to go down to the CCPACSSectionElement object, then call 
    # the function "get_ctigl_section_element" (see the code bellow).
    # Once we get the CTiglSectionElement object, we can set desired parameters.
    
    # Here we set the height and width of the section 2,3,4,18,19,20,21    
    for idx in (2,3,4,5,18,19,20,21) :
        s = fuselage.get_section(idx)
        e = s.get_section_element(1);
        ce = e.get_ctigl_section_element();
        ce.set_height(1.5)
        ce.set_width(2.3)

    # Here we set the area of the section the extremity  
    for idx in (1,22) :
        s = fuselage.get_section(idx)
        e = s.get_section_element(1);
        ce = e.get_ctigl_section_element();
        ce.set_area(0);
        
    return fuselage
    

def create_round_wing(aircraft,diameter, uid):

    numberOfSection = 23
    # rotation between each section 
    deltaRotX = 180.0  / (numberOfSection - 1.0)
    
    # Normaly, we should use CTiglPoint instead of gp.gp_Pnt
    # but there is for the moment a small issue in the binding of CTiglTransformation class
    firstPosition = OCC.gp.gp_Pnt(0,0,-diameter/2.0);  
    firstNormal = OCC.gp.gp_Pnt(0,-1,0);

    wings = aircraft.get_wings();
    # create the wing
    wings.create_wing(uid, numberOfSection , "NACA0006");
    wing = wings.get_wing(uid);
   
   
    rot =  tigl3.geometry.CTiglTransformation();
    # set the wing section elements
    for idx  in range(1,wing.get_section_count() + 1) :
        rotX = (idx - 1) * deltaRotX;
        rot.set_identity();
        rot.add_rotation_x(rotX);

        p = rot.transform(firstPosition);
        n = rot.transform(firstNormal);

        s = wing.get_section(idx);
        e = s.get_section_element(1);
        ce = e.get_ctigl_section_element();

        ce.set_center(tigl3.geometry.CTiglPoint(p.X(), p.Y(), p.Z()));
        ce.set_normal(tigl3.geometry.CTiglPoint(n.X(), n.Y(), n.Z()));
        if rotX >= 90:
            ce.set_rotation_around_normal(180);

    # set symmetry of the wing 
    sym = tigl3.geometry.TIGL_X_Z_PLANE 
    wing.set_symmetry(sym)
    
    return wing;




def create_space_ship():

    filename = "./data/empty.cpacs3.xml"
    # Open the file using tixi 
    tixi_h = tixi3.tixi3wrapper.Tixi3()
    tixi_h.open(filename)
    # then open the tigl handler passing the tixi hanler
    tigl_h = tigl3.tigl3wrapper.Tigl3()
    tigl_h.open(tixi_h, "")

    # To access the internal python api, you need to use the configuration manager
    mgr =  tigl3.configuration.CCPACSConfigurationManager_get_instance()
    aircraft = mgr.get_configuration(tigl_h._handle.value)
    
    # Now you have a CCPACSConfiguration object that is considered to be part 
    # of the internal python api. All the public functions available in 
    # the C++ CCPACSConfiguration class are also available for this python object. 
    # Remark that the function names change a little. 
    # In C++ the function names use camelCase and in python the function names
    # use snake_case. 
    
    # The following function use internal python api to create a fuselage
    fuselage = create_fuselage(aircraft, "Fuselage1")
    
    # The following function use internal python api to creare a wing 
    wing1 = create_round_wing(aircraft, 13, "Wing1")
    # All the function of the CCPACSWing class is available for the wing2 object
    # Especially, we can set the root leading edge position of the wing.
    wing1_position = wing1.get_root_leposition();
    wing1_position.x = 2
    wing1.set_root_leposition(wing1_position)
  
    # The following function use internal python api to creare a wing 
    wing2 = create_round_wing(aircraft, 23, "Wing2")
    wing2_position = wing2.get_root_leposition();
    wing2_position.x = 10
    wing2.set_root_leposition(wing2_position)
    
    wing3 = create_round_wing(aircraft, 13, "Wing3")
    wing3_position = wing3.get_root_leposition();
    wing3_position.x = 18
    wing3.set_root_leposition(wing3_position)

    aircraft.write_cpacs(aircraft.get_uid())
    config_as_string = tixi_h.exportDocumentAsString();
    text_file = open("./out.xml", "w")
    text_file.write(config_as_string)
    text_file.close()




if __name__ == '__main__':
    create_space_ship()
