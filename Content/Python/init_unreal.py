"""
UE5 Editor Python Initialization Script for HktMcpBridge

This script is loaded when the Unreal Editor starts (if configured in Project Settings).
It provides utilities for the MCP server to interact with the editor.

To enable:
1. Edit → Project Settings → Plugins → Python → Startup Scripts
2. Add: /Game/Python/init_unreal.py
"""

import unreal
import json
from typing import Any, Dict, List, Optional


# Get the MCP Editor Subsystem
def get_mcp_subsystem() -> Optional[unreal.HktMcpEditorSubsystem]:
    """Get the HktMcpEditorSubsystem instance"""
    try:
        return unreal.get_editor_subsystem(unreal.HktMcpEditorSubsystem)
    except Exception as e:
        unreal.log_error(f"[HktMcp] Failed to get MCP Subsystem: {e}")
        return None


# ==================== Utility Functions ====================

def log_info(message: str) -> None:
    """Log info message"""
    unreal.log(f"[HktMcp] {message}")


def log_warning(message: str) -> None:
    """Log warning message"""
    unreal.log_warning(f"[HktMcp] {message}")


def log_error(message: str) -> None:
    """Log error message"""
    unreal.log_error(f"[HktMcp] {message}")


def show_notification(message: str, duration: float = 3.0) -> None:
    """Show editor notification"""
    subsystem = get_mcp_subsystem()
    if subsystem:
        subsystem.show_notification(message, duration)
    else:
        log_warning(f"Notification (no subsystem): {message}")


# ==================== Asset Helpers ====================

def list_assets_in_path(path: str, class_filter: str = "") -> List[Dict]:
    """List assets in a path"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return []
    
    assets = subsystem.list_assets(path, class_filter)
    return [
        {
            "path": a.asset_path,
            "name": a.asset_name,
            "class": a.asset_class,
            "package": a.package_path
        }
        for a in assets
    ]


def search_assets_by_name(query: str, class_filter: str = "") -> List[Dict]:
    """Search assets by name"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return []
    
    assets = subsystem.search_assets(query, class_filter)
    return [
        {
            "path": a.asset_path,
            "name": a.asset_name,
            "class": a.asset_class
        }
        for a in assets
    ]


# ==================== Level Helpers ====================

def list_level_actors(class_filter: str = "") -> List[Dict]:
    """List actors in the current level"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return []
    
    actors = subsystem.list_actors(class_filter)
    return [
        {
            "name": a.actor_name,
            "label": a.actor_label,
            "class": a.actor_class,
            "location": {"x": a.location.x, "y": a.location.y, "z": a.location.z},
            "rotation": {"pitch": a.rotation.pitch, "yaw": a.rotation.yaw, "roll": a.rotation.roll}
        }
        for a in actors
    ]


def spawn_blueprint_actor(
    blueprint_path: str,
    location: tuple,
    rotation: tuple = (0, 0, 0),
    label: str = ""
) -> Optional[str]:
    """Spawn an actor from a blueprint"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return None
    
    loc = unreal.Vector(location[0], location[1], location[2])
    rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
    
    return subsystem.spawn_actor(blueprint_path, loc, rot, label)


def move_actor(actor_name: str, location: tuple) -> bool:
    """Move an actor to a new location"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return False
    
    # Get current transform
    actors = subsystem.list_actors("")
    actor = next((a for a in actors if a.actor_name == actor_name or a.actor_label == actor_name), None)
    
    if not actor:
        return False
    
    new_loc = unreal.Vector(location[0], location[1], location[2])
    current_rot = actor.rotation
    current_scale = actor.scale
    
    return subsystem.modify_actor_transform(actor_name, new_loc, current_rot, current_scale)


def delete_actor(actor_name: str) -> bool:
    """Delete an actor from the level"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return False
    
    return subsystem.delete_actor(actor_name)


# ==================== Editor Control ====================

def start_play() -> bool:
    """Start PIE session"""
    subsystem = get_mcp_subsystem()
    if subsystem:
        return subsystem.start_pie()
    return False


def stop_play() -> bool:
    """Stop PIE session"""
    subsystem = get_mcp_subsystem()
    if subsystem:
        return subsystem.stop_pie()
    return False


def is_playing() -> bool:
    """Check if PIE is running"""
    subsystem = get_mcp_subsystem()
    if subsystem:
        return subsystem.is_pie_running()
    return False


def execute_command(command: str) -> None:
    """Execute editor console command"""
    subsystem = get_mcp_subsystem()
    if subsystem:
        subsystem.execute_editor_command(command)


# ==================== Camera Control ====================

def get_camera() -> Dict:
    """Get viewport camera transform"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return {"error": "No subsystem"}
    
    location = unreal.Vector()
    rotation = unreal.Rotator()
    
    if subsystem.get_viewport_camera_transform(location, rotation):
        return {
            "location": {"x": location.x, "y": location.y, "z": location.z},
            "rotation": {"pitch": rotation.pitch, "yaw": rotation.yaw, "roll": rotation.roll}
        }
    return {"error": "Failed to get camera"}


def set_camera(location: tuple, rotation: tuple) -> bool:
    """Set viewport camera transform"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return False
    
    loc = unreal.Vector(location[0], location[1], location[2])
    rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
    
    return subsystem.set_viewport_camera_transform(loc, rot)


def look_at_actor(actor_name: str) -> bool:
    """Move camera to look at an actor"""
    subsystem = get_mcp_subsystem()
    if not subsystem:
        return False
    
    actors = subsystem.list_actors("")
    actor = next((a for a in actors if a.actor_name == actor_name or a.actor_label == actor_name), None)
    
    if not actor:
        return False
    
    # Position camera offset from actor
    loc = actor.location
    camera_loc = unreal.Vector(loc.x - 500, loc.y - 500, loc.z + 300)
    camera_rot = unreal.Rotator(-20, 45, 0)
    
    return subsystem.set_viewport_camera_transform(camera_loc, camera_rot)


# ==================== Initialization ====================

def on_editor_startup():
    """Called when editor starts"""
    log_info("HktMcpBridge Python utilities loaded")
    
    subsystem = get_mcp_subsystem()
    if subsystem:
        log_info("HktMcpEditorSubsystem is available")
        show_notification("HktMcpBridge Ready", 2.0)
    else:
        log_warning("HktMcpEditorSubsystem not available - check plugin")


# Run on load
try:
    on_editor_startup()
except Exception as e:
    log_error(f"Startup error: {e}")

