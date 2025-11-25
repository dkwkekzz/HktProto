"""
Query Tools - MCP tools for querying content in Unreal Engine 5

Provides tools for:
- Searching classes
- Getting class properties
- Project structure inspection
- Current level information
- Viewport camera queries
"""

import json
import logging
from typing import Any

from ..bridge.editor_bridge import EditorBridge

logger = logging.getLogger("hkt_mcp.tools.query")


async def search_classes(
    bridge: EditorBridge,
    query: str,
    blueprint_only: bool = False
) -> str:
    """
    Search for classes by name
    
    Args:
        bridge: Editor bridge instance
        query: Search query string
        blueprint_only: If True, search only Blueprint classes
    
    Returns:
        JSON string with class list
    """
    logger.info(f"Searching classes: {query} (blueprint_only: {blueprint_only})")
    
    classes = await bridge.search_classes(query, blueprint_only)
    
    result = {
        "query": query,
        "blueprint_only": blueprint_only,
        "count": len(classes),
        "classes": classes
    }
    
    return json.dumps(result, indent=2)


async def get_class_properties(bridge: EditorBridge, class_name: str) -> str:
    """
    Get all properties of a class
    
    Args:
        bridge: Editor bridge instance
        class_name: Name of the class
    
    Returns:
        JSON string with property list
    """
    logger.info(f"Getting properties of class: {class_name}")
    
    properties = await bridge.get_class_properties(class_name)
    
    result = {
        "class_name": class_name,
        "count": len(properties),
        "properties": properties
    }
    
    return json.dumps(result, indent=2)


async def get_project_structure(bridge: EditorBridge, root_path: str = "/Game") -> str:
    """
    Get project folder structure
    
    Args:
        bridge: Editor bridge instance
        root_path: Root path to start from
    
    Returns:
        JSON string with folder structure
    """
    logger.info(f"Getting project structure from: {root_path}")
    
    structure = await bridge.get_project_structure(root_path)
    
    # structure is already a JSON string
    return structure


async def get_level_info(bridge: EditorBridge) -> str:
    """
    Get information about the current level
    
    Args:
        bridge: Editor bridge instance
    
    Returns:
        JSON string with level information
    """
    logger.info("Getting current level info")
    
    info = await bridge.get_current_level_info()
    
    # info is already a JSON string
    return info


async def get_viewport_camera(bridge: EditorBridge) -> str:
    """
    Get current viewport camera position and rotation
    
    Args:
        bridge: Editor bridge instance
    
    Returns:
        JSON string with camera transform
    """
    logger.info("Getting viewport camera transform")
    
    camera_data = await bridge.get_viewport_camera()
    
    if "error" in camera_data:
        return json.dumps(camera_data)
    
    result = {
        "location": camera_data.get("location", {}),
        "rotation": camera_data.get("rotation", {}),
        "success": True
    }
    
    return json.dumps(result, indent=2)


async def show_notification(bridge: EditorBridge, message: str, duration: float = 3.0) -> str:
    """
    Show a notification in the editor
    
    Args:
        bridge: Editor bridge instance
        message: Message to display
        duration: Duration in seconds
    
    Returns:
        JSON string with result
    """
    logger.info(f"Showing notification: {message}")
    
    await bridge.show_notification(message, duration)
    
    result = {
        "message": message,
        "duration": duration,
        "success": True
    }
    
    return json.dumps(result, indent=2)


async def get_engine_version(bridge: EditorBridge) -> str:
    """
    Get Unreal Engine version
    
    Args:
        bridge: Editor bridge instance
    
    Returns:
        JSON string with engine version
    """
    logger.info("Getting engine version")
    
    project_info = await bridge.get_project_info()
    
    # project_info is already a JSON string
    return project_info


async def get_content_browser_path(bridge: EditorBridge) -> str:
    """
    Get current content browser selection path
    
    Args:
        bridge: Editor bridge instance
    
    Returns:
        JSON string with path
    """
    # This would need additional implementation
    result = {
        "path": "/Game",
        "message": "Content browser path query not fully implemented"
    }
    
    return json.dumps(result, indent=2)

