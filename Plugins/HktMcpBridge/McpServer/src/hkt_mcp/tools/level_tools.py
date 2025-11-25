"""
Level Tools - MCP tools for level editing in Unreal Engine 5

Provides tools for:
- Listing actors in the current level
- Spawning new actors
- Modifying actor transforms and properties
- Deleting actors
- Viewport camera control
"""

import json
import logging
from typing import Any, Optional

from ..bridge.editor_bridge import EditorBridge

logger = logging.getLogger("hkt_mcp.tools.level")


async def list_actors(bridge: EditorBridge, class_filter: str = "") -> str:
    """
    List all actors in the current level
    
    Args:
        bridge: Editor bridge instance
        class_filter: Optional class name to filter actors
    
    Returns:
        JSON string with actor list
    """
    logger.info(f"Listing actors (filter: {class_filter})")
    
    actors = await bridge.list_actors(class_filter)
    
    result = {
        "class_filter": class_filter,
        "count": len(actors),
        "actors": actors
    }
    
    return json.dumps(result, indent=2)


async def spawn_actor(
    bridge: EditorBridge,
    blueprint_path: str,
    location: tuple[float, float, float],
    rotation: tuple[float, float, float] = (0, 0, 0),
    label: str = ""
) -> str:
    """
    Spawn a new actor from a Blueprint
    
    Args:
        bridge: Editor bridge instance
        blueprint_path: Path to the Blueprint asset
        location: (x, y, z) spawn location
        rotation: (pitch, yaw, roll) spawn rotation
        label: Optional actor label
    
    Returns:
        JSON string with result
    """
    logger.info(f"Spawning actor from {blueprint_path} at {location}")
    
    actor_name = await bridge.spawn_actor(blueprint_path, location, rotation, label)
    
    result = {
        "blueprint_path": blueprint_path,
        "location": {"x": location[0], "y": location[1], "z": location[2]},
        "rotation": {"pitch": rotation[0], "yaw": rotation[1], "roll": rotation[2]},
        "label": label,
        "success": bool(actor_name),
        "actor_name": actor_name,
        "message": f"Spawned actor: {actor_name}" if actor_name else "Failed to spawn actor"
    }
    
    return json.dumps(result, indent=2)


async def spawn_actor_by_class(
    bridge: EditorBridge,
    class_name: str,
    location: tuple[float, float, float],
    rotation: tuple[float, float, float] = (0, 0, 0)
) -> str:
    """
    Spawn a new actor by native class name
    
    Args:
        bridge: Editor bridge instance
        class_name: Class name (e.g., 'PointLight', 'StaticMeshActor')
        location: (x, y, z) spawn location
        rotation: (pitch, yaw, roll) spawn rotation
    
    Returns:
        JSON string with result
    """
    logger.info(f"Spawning {class_name} at {location}")
    
    # This would use spawn_actor_by_class in the subsystem
    result = {
        "class_name": class_name,
        "location": {"x": location[0], "y": location[1], "z": location[2]},
        "rotation": {"pitch": rotation[0], "yaw": rotation[1], "roll": rotation[2]},
        "success": False,
        "message": "spawn_actor_by_class requires direct subsystem access"
    }
    
    return json.dumps(result, indent=2)


async def modify_actor(
    bridge: EditorBridge,
    actor_name: str,
    location: Optional[dict] = None,
    rotation: Optional[dict] = None,
    scale: Optional[dict] = None
) -> str:
    """
    Modify an actor's transform
    
    Args:
        bridge: Editor bridge instance
        actor_name: Name or label of the actor
        location: Optional new location {x, y, z}
        rotation: Optional new rotation {pitch, yaw, roll}
        scale: Optional new scale {x, y, z}
    
    Returns:
        JSON string with result
    """
    logger.info(f"Modifying actor {actor_name}")
    
    loc_tuple = (location["x"], location["y"], location["z"]) if location else None
    rot_tuple = (rotation["pitch"], rotation["yaw"], rotation["roll"]) if rotation else None
    scale_tuple = (scale["x"], scale["y"], scale["z"]) if scale else None
    
    success = await bridge.modify_actor_transform(actor_name, loc_tuple, rot_tuple, scale_tuple)
    
    result = {
        "actor_name": actor_name,
        "location": location,
        "rotation": rotation,
        "scale": scale,
        "success": success,
        "message": "Actor modified successfully" if success else "Failed to modify actor"
    }
    
    return json.dumps(result, indent=2)


async def modify_actor_property(
    bridge: EditorBridge,
    actor_name: str,
    property_name: str,
    new_value: str
) -> str:
    """
    Modify an actor's property
    
    Args:
        bridge: Editor bridge instance
        actor_name: Name or label of the actor
        property_name: Property to modify
        new_value: New value
    
    Returns:
        JSON string with result
    """
    logger.info(f"Modifying actor {actor_name} property {property_name}")
    
    # This would need to be implemented in the bridge
    result = {
        "actor_name": actor_name,
        "property_name": property_name,
        "new_value": new_value,
        "success": False,
        "message": "modify_actor_property not fully implemented"
    }
    
    return json.dumps(result, indent=2)


async def delete_actor(bridge: EditorBridge, actor_name: str) -> str:
    """
    Delete an actor from the level
    
    Args:
        bridge: Editor bridge instance
        actor_name: Name or label of the actor
    
    Returns:
        JSON string with result
    """
    logger.info(f"Deleting actor: {actor_name}")
    
    success = await bridge.delete_actor(actor_name)
    
    result = {
        "actor_name": actor_name,
        "success": success,
        "message": f"Deleted actor: {actor_name}" if success else "Failed to delete actor"
    }
    
    return json.dumps(result, indent=2)


async def select_actor(bridge: EditorBridge, actor_name: str) -> str:
    """
    Select an actor in the editor
    
    Args:
        bridge: Editor bridge instance
        actor_name: Name or label of the actor
    
    Returns:
        JSON string with result
    """
    logger.info(f"Selecting actor: {actor_name}")
    
    success = await bridge.select_actor(actor_name)
    
    result = {
        "actor_name": actor_name,
        "success": success,
        "message": f"Selected actor: {actor_name}" if success else "Failed to select actor"
    }
    
    return json.dumps(result, indent=2)


async def get_selected_actors(bridge: EditorBridge) -> str:
    """
    Get currently selected actors
    
    Args:
        bridge: Editor bridge instance
    
    Returns:
        JSON string with selected actors
    """
    # This would need to be implemented in the bridge
    result = {
        "count": 0,
        "actors": [],
        "message": "get_selected_actors not fully implemented"
    }
    
    return json.dumps(result, indent=2)


async def set_viewport_camera(
    bridge: EditorBridge,
    location: tuple[float, float, float],
    rotation: tuple[float, float, float]
) -> str:
    """
    Set viewport camera transform
    
    Args:
        bridge: Editor bridge instance
        location: (x, y, z) camera location
        rotation: (pitch, yaw, roll) camera rotation
    
    Returns:
        JSON string with result
    """
    logger.info(f"Setting viewport camera to {location}")
    
    success = await bridge.set_viewport_camera(location, rotation)
    
    result = {
        "location": {"x": location[0], "y": location[1], "z": location[2]},
        "rotation": {"pitch": rotation[0], "yaw": rotation[1], "roll": rotation[2]},
        "success": success,
        "message": "Camera moved successfully" if success else "Failed to move camera"
    }
    
    return json.dumps(result, indent=2)


async def focus_on_actor(bridge: EditorBridge, actor_name: str) -> str:
    """
    Focus viewport camera on an actor
    
    Args:
        bridge: Editor bridge instance
        actor_name: Name or label of the actor
    
    Returns:
        JSON string with result
    """
    logger.info(f"Focusing on actor: {actor_name}")
    
    # First select the actor
    select_success = await bridge.select_actor(actor_name)
    
    # Then get actor location and move camera near it
    actors = await bridge.list_actors()
    actor = next((a for a in actors if a["actor_name"] == actor_name or a["actor_label"] == actor_name), None)
    
    if actor:
        loc = actor["location"]
        # Position camera slightly offset from actor
        camera_loc = (loc["x"] - 500, loc["y"] - 500, loc["z"] + 200)
        camera_rot = (20, 45, 0)  # Looking down and towards
        await bridge.set_viewport_camera(camera_loc, camera_rot)
    
    result = {
        "actor_name": actor_name,
        "success": actor is not None,
        "message": f"Focused on actor: {actor_name}" if actor else "Actor not found"
    }
    
    return json.dumps(result, indent=2)

