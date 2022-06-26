import bpy


newBone = None

def addBone():
    """Add the origin bone and move to the origin point."""
    bpy.ops.object.mode_set(mode='EDIT')
    
    bpy.ops.armature.bone_primitive_add(name="Origin")
    
    newBone = bpy.context.selected_objects[0].data.edit_bones['Origin']
    
    if newBone is None:
        print("Failed to create bone")
        return
    
    # Deselect everything
    bpy.ops.armature.select_all(action='DESELECT')
    
    # Scale up the bone so we can see it. The head should remain at (0,0,0)
    newBone.select_tail = True
    bpy.ops.transform.translate(value=(0, 0, 10))
    
    
def copyKeyframes():
    """Copies keyframes of Hip bone to the Origin bone"""
    
    print("Copy keyframes")
    
    # select the origin bone
    newBone = bpy.context.selected_objects[0].data.edit_bones['Origin']
    
    if not newBone:
        print("Could not find the hip bone")
        return
    
    newBone.select = True
    
    # First get into pose mode
    bpy.ops.object.mode_set(mode='POSE')
    
    bone = bpy.context.selected_pose_bones[0]
    obj = bpy.context.selected_objects[0]
    
    for action in bpy.data.actions:
        print('analyzing action %s with %d fcurves' % (action, len(action.fcurves)))
        
        fd = [[], [], []]
        for fcurve in action.fcurves:
            if 'Hips' not in fcurve.data_path or 'location' not in fcurve.data_path:
                continue
            for k in fcurve.keyframe_points:
                fd[fcurve.array_index].append(k.co)
            
        obj.animation_data.action = action
        
        # create the keyframes
        for index in range(len(fd[0])):
            frame = fd[0][index][0]
            vec = (fd[0][index][1], fd[1][index][1], fd[2][index][1])
            print("Vec = ", vec)
            bone.location = (vec[0], 0, vec[2])
            bone.keyframe_insert(data_path="location", frame=frame)
            #print("vec = ", vec, ", frame = ", frame)
    

class MixamoOriginBoneFixup(bpy.types.Operator):
    """Tooltip"""
    bl_idname = "object.simple_operator"
    bl_label = "Mixamo Origin Bone Fixup"

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        if bpy.context.active_object is None:
            return

        selected = bpy.context.selected_objects

        if selected:
            armature = selected[0]
            print("Selected armature =", armature)
            if "Origin" not in armature.data.bones:
                addBone()
            else:
                print("Already added bone, 'Origin'")
            copyKeyframes()
        
        return {'FINISHED'}


def menu_func(self, context):
    self.layout.operator(MixamoOriginBoneFixup.bl_idname, text=SimpleOperator.bl_label)


# Register and add to the "object" menu (required to also use F3 search "Simple Object Operator" for quick access).
def register():
    bpy.utils.register_class(MixamoOriginBoneFixup)
    bpy.types.VIEW3D_MT_object.append(menu_func)


def unregister():
    bpy.utils.unregister_class(MixamoOriginBoneFixup)
    bpy.types.VIEW3D_MT_object.remove(menu_func)


if __name__ == "__main__":
    register()

    bpy.ops.object.simple_operator()