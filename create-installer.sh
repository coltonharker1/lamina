#!/bin/bash

# Lamina Plugin Installer Creator
# Creates a professional .pkg installer for macOS that auto-installs plugins

set -e  # Exit on any error

echo "🎵 Creating Lamina Installer..."

# Configuration
PLUGIN_NAME="Lamina"
VERSION="1.0"
IDENTIFIER="com.projectsonorous.lamina"
BUILD_DIR="$(pwd)/build/GrainsVST_artefacts/Release"
INSTALLER_DIR="$(pwd)/installer"
PAYLOAD_DIR="$INSTALLER_DIR/payload"
SCRIPTS_DIR="$INSTALLER_DIR/scripts"
OUTPUT_DIR="$(pwd)/installer-output"

# Clean up old builds
echo "🧹 Cleaning up old installer files..."
rm -rf "$INSTALLER_DIR" "$OUTPUT_DIR"
mkdir -p "$PAYLOAD_DIR" "$SCRIPTS_DIR" "$OUTPUT_DIR"

# Create directory structure for installation
echo "📦 Preparing plugin files..."

# AU destination: /Library/Audio/Plug-Ins/Components/
mkdir -p "$PAYLOAD_DIR/Library/Audio/Plug-Ins/Components"
if [ -d "$BUILD_DIR/AU/Lamina.component" ]; then
    cp -R "$BUILD_DIR/AU/Lamina.component" "$PAYLOAD_DIR/Library/Audio/Plug-Ins/Components/"
    echo "  ✅ Audio Unit (AU) added"
else
    echo "  ⚠️  AU not found, skipping..."
fi

# VST3 destination: /Library/Audio/Plug-Ins/VST3/
mkdir -p "$PAYLOAD_DIR/Library/Audio/Plug-Ins/VST3"
if [ -d "$BUILD_DIR/VST3/Lamina.vst3" ]; then
    cp -R "$BUILD_DIR/VST3/Lamina.vst3" "$PAYLOAD_DIR/Library/Audio/Plug-Ins/VST3/"
    echo "  ✅ VST3 added"
else
    echo "  ⚠️  VST3 not found, skipping..."
fi

# Create welcome message
cat > "$INSTALLER_DIR/welcome.txt" << 'EOF'
Welcome to Lamina v1.0

Lamina is a professional grain synthesis instrument for music production.

This installer will install:
• Audio Unit (AU) for Logic Pro, GarageBand
• VST3 for Ableton Live, FL Studio, and other DAWs

Installation locations:
• /Library/Audio/Plug-Ins/Components/Lamina.component
• /Library/Audio/Plug-Ins/VST3/Lamina.vst3

After installation, restart your DAW or rescan plugins.

Project Sonorous © 2024
EOF

# Create post-install script (fixes permissions)
cat > "$SCRIPTS_DIR/postinstall" << 'EOF'
#!/bin/bash

# Fix permissions after installation
chmod -R 755 /Library/Audio/Plug-Ins/Components/Lamina.component
chmod -R 755 /Library/Audio/Plug-Ins/VST3/Lamina.vst3

# Clear audio plugin cache to force rescan
rm -rf ~/Library/Caches/AudioUnitCache 2>/dev/null || true

echo "Lamina installed successfully!"
exit 0
EOF

chmod +x "$SCRIPTS_DIR/postinstall"

# Build the package
echo "🔨 Building installer package..."

pkgbuild --root "$PAYLOAD_DIR" \
         --identifier "$IDENTIFIER" \
         --version "$VERSION" \
         --scripts "$SCRIPTS_DIR" \
         --install-location "/" \
         "$INSTALLER_DIR/Lamina-component.pkg"

# Create distribution XML for productbuild (adds welcome screen, customization)
cat > "$INSTALLER_DIR/distribution.xml" << EOF
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>Lamina v$VERSION</title>
    <organization>com.projectsonorous</organization>
    <welcome file="welcome.txt"/>
    <options customize="never" require-scripts="false" hostArchitectures="arm64,x86_64"/>
    <domains enable_localSystem="true"/>

    <choices-outline>
        <line choice="default">
            <line choice="com.projectsonorous.lamina"/>
        </line>
    </choices-outline>

    <choice id="default"/>
    <choice id="com.projectsonorous.lamina" visible="false">
        <pkg-ref id="$IDENTIFIER"/>
    </choice>

    <pkg-ref id="$IDENTIFIER" version="$VERSION" onConclusion="none">Lamina-component.pkg</pkg-ref>
</installer-gui-script>
EOF

# Create final installer with custom UI
echo "🎨 Creating final installer with UI..."

productbuild --distribution "$INSTALLER_DIR/distribution.xml" \
             --resources "$INSTALLER_DIR" \
             --package-path "$INSTALLER_DIR" \
             "$OUTPUT_DIR/Lamina-v$VERSION-macOS.pkg"

# Get file size
INSTALLER_SIZE=$(du -h "$OUTPUT_DIR/Lamina-v$VERSION-macOS.pkg" | cut -f1)

echo ""
echo "✨ Installer created successfully!"
echo "📍 Location: $OUTPUT_DIR/Lamina-v$VERSION-macOS.pkg"
echo "📦 Size: $INSTALLER_SIZE"
echo ""
echo "🧪 Test the installer:"
echo "   sudo installer -pkg \"$OUTPUT_DIR/Lamina-v$VERSION-macOS.pkg\" -target /"
echo ""
echo "📤 Upload this file to your website for distribution!"
