from tixi3.tixi3wrapper import Tixi3
from tigl3.tigl3wrapper import Tigl3
import tigl3.configuration

tixi_h = Tixi3()
tigl_h = Tigl3()

tixi_h.open("hilfe_ohne_build_in_pixi.cpacs.xml")
tigl_h.open(tixi_h, "")

mgr = tigl3.configuration.CCPACSConfigurationManager_get_instance()
aircraft_config = mgr.get_configuration(tigl_h._handle.value)
wing = aircraft_config.get_wing("wing")
print(aircraft_config.get_wing_count())





help(wing)