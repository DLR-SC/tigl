package de.dlr.sc.tiglviewer.android;

public class ParentItem extends Item {


    public ParentItem(String name)
    {
        this.name = name;
    }
    
    @Override
    public int isParentSection() {
        return 1;
    }


}
