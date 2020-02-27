/*
 * Graph.h
 *
 * Copyright (C) 2019 by Universitaet Stuttgart (VIS).
 * Alle Rechte vorbehalten.
 */

#ifndef MEGAMOL_GUI_GRAPH_GRAPH_H_INCLUDED
#define MEGAMOL_GUI_GRAPH_GRAPH_H_INCLUDED

#include "vislib/sys/Log.h"

#include <map>
#include <vector>

#include "Call.h"
#include "CallSlot.h"
#include "Module.h"
#include "Parameter.h"
#include "Stock.h"


namespace megamol {
namespace gui {
namespace graph {

class Graph {
public:
    typedef std::vector<ModulePtrType> ModuleGraphVectorType;
    typedef std::vector<CallPtrType> CallGraphVectorType;

    Graph(const std::string& graph_name);

    virtual ~Graph(void);

    bool AddModule(const ModuleStockType& stock_modules, const std::string& module_class_name);
    bool DeleteModule(int module_uid);

    bool AddCall(
        const CallStockType& stock_calls, int call_idx, CallSlotPtrType call_slot_1, CallSlotPtrType call_slot_2);
    bool DeleteDisconnectedCalls(void);
    bool DeleteCall(int call_uid);

    const const ModuleGraphVectorType& GetGraphModules(void) { return this->modules; }
    const const CallGraphVectorType& GetGraphCalls(void) { return this->calls; }

    inline void SetName(const std::string& graph_name) { this->name = graph_name; }
    inline const std::string& GetName(void) { return this->name; }

    inline bool IsDirty(void) const { return this->dirty_flag; }
    inline void ResetDirty(void) { this->dirty_flag = false; }

    inline int GetUID(void) const { return this->uid; }

    int generate_unique_id(void) { return (++this->generated_uid); }

    // GUI Presentation -------------------------------------------------------
    bool Present(void) { this->present.Present(*this); }

    inline const CallSlotPtrType GetSelectedSlot(void) const {
        return this->present.selected_slot_ptr;
    }

private:
    // VARIABLES --------------------------------------------------------------

    ModuleGraphVectorType modules;
    CallGraphVectorType calls;

    // UIDs are unique within a graph
    const int uid;
    std::string name;
    bool dirty_flag;

    // Global variable for unique id shared/accessible by all graphs.
    static int generated_uid;

    /**
     * Defines GUI graph present.
     */
    class Presentation {
    public:
        Presentation(void);

        ~Presentation(void);

        bool Present(Graph& graph);

        float slot_radius;
        ImVec2 canvas_position;
        ImVec2 canvas_size;
        ImVec2 canvas_scrolling;
        float canvas_zooming;
        ImVec2 canvas_offset;
        bool show_grid;
        bool show_call_names;
        bool show_slot_names;
        int selected_module_uid;
        int selected_call_uid;
        int hovered_slot_uid;
        CallSlotPtrType selected_slot_ptr;
        int process_selected_slot;

    private:

        GUIUtils utils;

    } present;
};

} // namespace graph
} // namespace gui
} // namespace megamol

#endif // MEGAMOL_GUI_GRAPH_GRAPH_H_INCLUDED