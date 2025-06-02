async function handleDelete(todoId) {
    const res = await fetch(`del.py?todoId=${todoId}`, {
        method: 'DELETE'
    })
    if (!res.ok)
        return window.location = 'app.py?error=1'
    return window.location = 'app.py?success=1'
}
