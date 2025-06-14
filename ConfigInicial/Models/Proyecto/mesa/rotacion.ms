/* Script para rotar todas las mesas clonadas 90� alrededor del eje Y */

for i = 1 to 20 do (
    local nodeName = "mesa" + i as string
    local mesaNode = getNodeByName nodeName

    if mesaNode != undefined then (
        -- Aplicar rotaci�n absoluta de 90� en Y
        mesaNode.rotation = eulerAngles 0 90 0

        -- Si prefieres rotarlo relativo a su rotaci�n actual, usa:
        -- mesaNode.rotation *= eulerAngles 0 90 0

    ) else (
        format "?? No se encontr� el nodo %\n" nodeName
    )
)

print "?? Todas las mesas han sido rotadas 90� en Y."
