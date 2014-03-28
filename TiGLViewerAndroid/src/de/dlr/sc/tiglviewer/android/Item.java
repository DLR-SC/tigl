package de.dlr.sc.tiglviewer.android;

public abstract class Item 
{   
    public String name;
    public int isParentSection()
    {
        return -1;
    }
    public String getName()
    {
        return this.name;
    }

}
