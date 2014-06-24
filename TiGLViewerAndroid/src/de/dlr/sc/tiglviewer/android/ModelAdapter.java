/*
 * Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package de.dlr.sc.tiglviewer.android;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class ModelAdapter extends ArrayAdapter<Item> {

    private ArrayList<Item> items;
    private LayoutInflater vi;

    private enum fieldType {
        PARENT_ITEM, CHILD_ITEM
    }

    public ModelAdapter(Context context, ArrayList<Item> items) {
        super(context, 0, items);
        this.items = items;
        vi = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getViewTypeCount() {
        return fieldType.values().length;
    }

    @Override
    public int getItemViewType(int index) {

        if (getItem(index).isParentSection() == 1) {
            return 1;
        } else {
            return 0;
        }
    }

    @Override
    public boolean areAllItemsEnabled() {
        return false;
    }

    @Override
    public boolean isEnabled(int index) {
        if (index == 0) {
            return false;
        } else {
            return true;
        }
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View v = convertView;
        final Item i = items.get(position);

        if (i != null) {
            if (i.isParentSection() == 1) {
                ParentItem p = (ParentItem) i;
                v = vi.inflate(R.layout.drawer_list_item, null);

                v.setOnClickListener(null);
                v.setOnLongClickListener(null);
                v.setLongClickable(false);

                final TextView parentView = (TextView) v
                        .findViewById(R.id.parent);
                parentView.setText(p.name);
            } else {
                ChildItem c = (ChildItem) i;
                v = vi.inflate(R.layout.drawer_list_item_1, null);
                final TextView name = (TextView) v.findViewById(R.id.child);
                if (name != null) {
                    /*if (c.name == "Export File") {
                        v.setOnClickListener(null);
                        v.setOnLongClickListener(null);
                        v.setLongClickable(false);
                    }*/
                    name.setText(c.name);
                }
            }
        }
        return v;
    }

}
