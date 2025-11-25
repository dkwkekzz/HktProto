"""
Asset Tools - MCP tools for asset management in Unreal Engine 5

Provides tools for:
- Listing assets by path and class
- Searching assets by name
- Getting asset details
- Modifying asset properties
"""

import json
import logging
from typing import Any

from ..bridge.editor_bridge import EditorBridge

logger = logging.getLogger("hkt_mcp.tools.asset")


async def list_assets(bridge: EditorBridge, path: str, class_filter: str = "") -> str:
    """
    List assets in a specified path
    
    Args:
        bridge: Editor bridge instance
        path: Asset path (e.g., '/Game/Blueprints')
        class_filter: Optional class name to filter (e.g., 'Blueprint')
    
    Returns:
        JSON string with asset list
    """
    logger.info(f"Listing assets in {path} (filter: {class_filter})")
    
    assets = await bridge.list_assets(path, class_filter)
    
    result = {
        "path": path,
        "class_filter": class_filter,
        "count": len(assets),
        "assets": assets
    }
    
    return json.dumps(result, indent=2)


async def get_asset_info(bridge: EditorBridge, asset_path: str) -> str:
    """
    Get detailed information about an asset
    
    Args:
        bridge: Editor bridge instance
        asset_path: Full asset path
    
    Returns:
        JSON string with asset details
    """
    logger.info(f"Getting asset info: {asset_path}")
    
    details = await bridge.get_asset_details(asset_path)
    
    # details is already a JSON string
    return details


async def search_assets(bridge: EditorBridge, query: str, class_filter: str = "") -> str:
    """
    Search for assets by name
    
    Args:
        bridge: Editor bridge instance
        query: Search query string
        class_filter: Optional class filter
    
    Returns:
        JSON string with search results
    """
    logger.info(f"Searching assets: {query} (filter: {class_filter})")
    
    assets = await bridge.search_assets(query, class_filter)
    
    result = {
        "query": query,
        "class_filter": class_filter,
        "count": len(assets),
        "assets": assets
    }
    
    return json.dumps(result, indent=2)


async def modify_asset(
    bridge: EditorBridge,
    asset_path: str,
    property_name: str,
    new_value: str
) -> str:
    """
    Modify an asset property
    
    Args:
        bridge: Editor bridge instance
        asset_path: Full asset path
        property_name: Name of the property to modify
        new_value: New value for the property
    
    Returns:
        JSON string with result
    """
    logger.info(f"Modifying asset {asset_path}: {property_name} = {new_value}")
    
    success = await bridge.modify_asset_property(asset_path, property_name, new_value)
    
    result = {
        "asset_path": asset_path,
        "property_name": property_name,
        "new_value": new_value,
        "success": success,
        "message": "Property modified successfully" if success else "Failed to modify property"
    }
    
    return json.dumps(result, indent=2)


async def delete_asset(bridge: EditorBridge, asset_path: str) -> str:
    """
    Delete an asset
    
    Args:
        bridge: Editor bridge instance
        asset_path: Full asset path
    
    Returns:
        JSON string with result
    """
    logger.info(f"Deleting asset: {asset_path}")
    
    # This would need to be implemented in the bridge
    # For now, return a placeholder
    result = {
        "asset_path": asset_path,
        "success": False,
        "message": "Delete asset not implemented yet"
    }
    
    return json.dumps(result, indent=2)


async def duplicate_asset(
    bridge: EditorBridge,
    source_path: str,
    destination_path: str
) -> str:
    """
    Duplicate an asset
    
    Args:
        bridge: Editor bridge instance
        source_path: Source asset path
        destination_path: Destination asset path
    
    Returns:
        JSON string with result
    """
    logger.info(f"Duplicating asset: {source_path} -> {destination_path}")
    
    # This would need to be implemented in the bridge
    result = {
        "source_path": source_path,
        "destination_path": destination_path,
        "success": False,
        "message": "Duplicate asset not implemented yet"
    }
    
    return json.dumps(result, indent=2)


async def create_data_asset(
    bridge: EditorBridge,
    asset_path: str,
    parent_class: str
) -> str:
    """
    Create a new DataAsset
    
    Args:
        bridge: Editor bridge instance
        asset_path: Path for the new asset
        parent_class: Parent class name
    
    Returns:
        JSON string with result
    """
    logger.info(f"Creating DataAsset: {asset_path} ({parent_class})")
    
    # This would need to be implemented in the bridge
    result = {
        "asset_path": asset_path,
        "parent_class": parent_class,
        "success": False,
        "message": "Create DataAsset not implemented yet"
    }
    
    return json.dumps(result, indent=2)

